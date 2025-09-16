#!/usr/bin/env python3
"""
Agent Hub for MoreMojo project - manages swarm of specialized agents for build fixes and builds.
Implements an intelligent agent system that can detect and fix issues in the MoreMojo codebase,
as well as building the app and plugin components.
"""

import os
import re
import subprocess
import sys
import logging
import argparse
import pathlib
import shutil
import json
from typing import Dict, List, Any, Tuple

# Setup paths
ROOT = pathlib.Path(__file__).resolve().parents[2]
SRC  = ROOT / "app" / "Sources"
PLUGIN = ROOT / "plugin"
LOGS_DL = ROOT / "failed_artifacts"   # downloaded artifacts
LOGS_CI = ROOT / "ci_logs"            # inline logs from build job
BUILD_DIR = ROOT / "build"            # Local build directory
SUMMARY = ROOT / "swarm_summary.md"

# Set up logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(),
        logging.FileHandler(ROOT / "swarm.log"),
    ]
)
logger = logging.getLogger("AgentHub")

# Import the agent registry and agents
try:
    from agents import agent_registry
    from agents.build_agent import BuildAgent
except ImportError:
    # If running directly from this file, adjust the import path
    sys.path.insert(0, str(pathlib.Path(__file__).parent))
    from agents import agent_registry
    from agents.build_agent import BuildAgent

def sh(cmd, check=True):
    """Run a shell command."""
    logger.info(f"$ {cmd}")
    p = subprocess.run(cmd, shell=True, text=True, capture_output=True)
    if check and p.returncode != 0:
        logger.error(f"Command failed with code {p.returncode}: {p.stderr}")
        raise RuntimeError(f"Command failed: {cmd}")
    return p.returncode, p.stdout, p.stderr

def write(p: pathlib.Path, s: str) -> bool:
    """Write content to a file if it's different from current content."""
    p.parent.mkdir(parents=True, exist_ok=True)
    old = p.read_text(errors="ignore") if p.exists() else ""
    if old == s: return False
    p.write_text(s)
    logger.info(f"Wrote {p}")
    return True

def stage_all(): 
    """Stage all changes to git."""
    sh("git add -A", check=False)

def has_changes() -> bool: 
    """Check if there are any git changes."""
    return subprocess.run("git diff --quiet", shell=True).returncode != 0

def read_globs(*globs):
    """Read contents of files matching glob patterns."""
    buf = ""
    for g in globs:
        for p in ROOT.glob(g):
            try: buf += p.read_text(errors="ignore")
            except: pass
    return buf

def first_match_excerpt(text:str, pattern:str, lines=20):
    """Find a pattern in text and return surrounding lines."""
    try:
        m = re.search(pattern, text, flags=re.IGNORECASE|re.MULTILINE)
        if not m: return ""
        start = max(0, text.rfind("\n", 0, m.start() - 1))
        end   = text.find("\n", m.end())
        start = 0 if start < 0 else start
        end   = len(text) if end < 0 else end
        # expand to N lines around
        before = text[:m.start()].splitlines()[-lines:]
        after  = text[m.end():].splitlines()[:lines]
        return "\n".join(before + [text[m.start():m.end()]] + after)
    except Exception:
        return ""

def run_agents(app_log, plugin_log):
    """Run all agents that want to run and return their decisions."""
    decisions = {}
    changes_made = False
    
    # Instantiate all agents
    agents = agent_registry.instantiate_agents(ROOT)
    
    # Run each agent that wants to run
    for agent in agents:
        name = agent.__class__.name()
        wants_to_run = agent.wants(app_log, plugin_log)
        decisions[name] = wants_to_run
        
        if wants_to_run:
            logger.info(f"Running {name}...")
            try:
                agent_changed = agent.run()
                if agent_changed:
                    changes_made = True
                    logger.info(f"{name} made changes")
                else:
                    logger.info(f"{name} did not make any changes")
            except Exception as e:
                logger.error(f"Error running {name}: {e}", exc_info=True)
    
    # Stage all changes
    if changes_made:
        stage_all()
    
    return decisions, changes_made

def build(args):
    """Build the app and plugin."""
    build_agent = BuildAgent(ROOT)
    
    if args.app:
        logger.info("Building app...")
        app_success = build_agent.build_app()
        if not app_success:
            logger.error("App build failed")
            return False
        logger.info("App build succeeded!")
    
    if args.plugin:
        logger.info("Building plugin...")
        plugin_success = build_agent.build_plugin()
        if not plugin_success:
            logger.error("Plugin build failed")
            return False
        logger.info("Plugin build succeeded!")
    
    if args.all:
        logger.info("Building all components...")
        app_success, plugin_success = build_agent.build_all()
        if not app_success or not plugin_success:
            logger.error("Build failed")
            return False
        logger.info("All builds succeeded!")
    
    return True

def main():
    parser = argparse.ArgumentParser(description="MoreMojo Agent Hub")
    parser.add_argument("--fix", action="store_true", help="Run agents to fix issues")
    parser.add_argument("--build", action="store_true", help="Build app and plugin")
    parser.add_argument("--app", action="store_true", help="Build only the app")
    parser.add_argument("--plugin", action="store_true", help="Build only the plugin")
    parser.add_argument("--all", action="store_true", help="Build both app and plugin")
    parser.add_argument("--local-logs", action="store_true", help="Use local logs instead of downloading from CI")
    
    args = parser.parse_args()
    
    # Default to fix if no action specified
    if not any([args.fix, args.build, args.app, args.plugin, args.all]):
        args.fix = True
    
    # read logs from artifacts and inline
    app_log = read_globs("ci_logs/xcodebuild_app_stdout.log",
                        "ci_logs/app_preflight.txt",
                        "failed_artifacts/app/**/xcodebuild_app_stdout.log",
                        "failed_artifacts/app/**/app_preflight.txt",
                        "failed_artifacts/app/**/*.log")
    plugin_log = read_globs("ci_logs/cmake_configure.log",
                           "ci_logs/cmake_build.log",
                           "failed_artifacts/plugin/**/cmake_configure.log",
                           "failed_artifacts/plugin/**/cmake_build.log",
                           "failed_artifacts/plugin/**/CMake*.log")
    
    if args.fix:
        logger.info("Running agents to fix issues...")
        decisions, changes_made = run_agents(app_log, plugin_log)
        
        # Create summary
        summary = ["# Swarm Agent Hub Summary"]
        
        # Add decisions
        summary.append("## Agent Decisions")
        for agent_name, wanted in decisions.items():
            summary.append(f"- {agent_name}: {'YES' if wanted else 'no'}")
        
        # Add log info
        summary += [
            "",
            "## Log Information",
            f"- App log size: {len(app_log)} bytes",
            f"- Plugin log size: {len(plugin_log)} bytes",
            "",
            f"Changes made: {'YES' if changes_made else 'no'}"
        ]
        
        # Save summary
        write(SUMMARY, "\n".join(summary))
        logger.info("\n".join(summary))
        
        # If changes were made, ask about building
        if changes_made and not any([args.build, args.app, args.plugin, args.all]):
            print("\nChanges were made. Would you like to build now? (y/n)")
            choice = input().strip().lower()
            if choice in ['y', 'yes']:
                args.all = True
    
    # Handle build requests
    if any([args.build, args.app, args.plugin, args.all]):
        # If no specific build target is specified but --build is, build everything
        if args.build and not any([args.app, args.plugin, args.all]):
            args.all = True
        
        build_success = build(args)
        if not build_success:
            sys.exit(1)
    
    # Show changes
    logger.info("Changes detected by Swarm:")
    _, stdout, _ = sh("git -C \"" + str(ROOT) + "\" status --porcelain=v1", check=False)
    logger.info(stdout)
    _, stdout, _ = sh("git -C \"" + str(ROOT) + "\" diff --stat", check=False)
    logger.info(stdout)
    
    if has_changes():
        logger.info("Swarm agents made changes! Review the diff above.")
        print("\nTo commit these changes:")
        print("  git add -A")
        print("  git commit -m 'Swarm: <description of fixes>'")
        print("  git push -u origin <branch>")
    else:
        logger.info("No changes made by Swarm agents")
    
    sys.exit(0)

if __name__ == "__main__":
    main()
