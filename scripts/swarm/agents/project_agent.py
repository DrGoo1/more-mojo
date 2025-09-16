#!/usr/bin/env python3
"""Project agent to fix future Xcode project format issues."""

import os
import subprocess
import shutil
from pathlib import Path

from .base_agent import BaseAgent
from .registry import register_agent

@register_agent
class ProjectAgent(BaseAgent):
    """Fix future project format via XcodeGen."""
    
    PATTERNS = [
        "future Xcode project file format",
        "Unable to read project",
        "cannot be opened because it is in a future Xcode project file format",
    ]
    
    @classmethod
    def name(cls) -> str:
        return "ProjectAgent"
    
    @classmethod
    def description(cls) -> str:
        return "Fixes issues with future Xcode project formats by regenerating via XcodeGen"
    
    def wants(self, app_logs: str, plugin_logs: str) -> bool:
        """Determine if the agent wants to run based on log content."""
        return any(pat in app_logs for pat in self.PATTERNS)
    
    def run(self) -> bool:
        """Fix the Xcode project format issues by regenerating with XcodeGen."""
        self.logger.info("XcodeGen regeneration (if app/project.yml exists)")
        
        # Install xcodegen if needed
        try:
            subprocess.run("which xcodegen", shell=True, check=True, capture_output=True)
        except subprocess.CalledProcessError:
            self.logger.info("Installing XcodeGen...")
            subprocess.run("brew update || true", shell=True, check=False)
            subprocess.run("brew install xcodegen", shell=True, check=False)
        
        # Check if project.yml exists
        projy = self.root / "app" / "project.yml"
        if not projy.exists():
            self.logger.warning("No project.yml found, cannot regenerate project")
            return False
        
        # Backup the existing project if it exists
        proj_path = self.root / "app" / "MoreMojoStudio.xcodeproj"
        if proj_path.exists():
            backup_path = proj_path.with_suffix(".xcodeproj.bak")
            self.logger.info(f"Backing up existing project to {backup_path}")
            if backup_path.exists():
                shutil.rmtree(backup_path)
            shutil.copytree(proj_path, backup_path)
        
        # Run XcodeGen
        self.logger.info("Running XcodeGen...")
        app_path = self.root / "app"
        result = subprocess.run(
            f"cd \"{app_path}\" && xcodegen generate",
            shell=True,
            capture_output=True,
            text=True
        )
        
        if result.returncode == 0:
            self.logger.info("XcodeGen completed successfully")
            return True
        else:
            self.logger.error(f"XcodeGen failed: {result.stderr}")
            return False
