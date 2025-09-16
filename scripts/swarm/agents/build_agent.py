#!/usr/bin/env python3
"""Build agent to handle app and plugin builds."""

import os
import subprocess
import logging
import time
from pathlib import Path

from .base_agent import BaseAgent
from .registry import register_agent

@register_agent
class BuildAgent(BaseAgent):
    """Handle building of app and plugin components."""
    
    @classmethod
    def name(cls) -> str:
        return "BuildAgent"
    
    @classmethod
    def description(cls) -> str:
        return "Handles building the app and plugin components"
    
    def wants(self, app_logs: str, plugin_logs: str) -> bool:
        """Always returns True since this agent handles builds."""
        return True
    
    def run(self) -> bool:
        """This method doesn't modify files but runs builds."""
        self.logger.info("BuildAgent doesn't modify files directly, use build_app() and build_plugin() methods")
        return False
    
    def build_app(self) -> bool:
        """Build the macOS app."""
        self.logger.info("Building macOS app...")
        
        # Set up build directories
        build_dir = self.root / "build" / "app"
        build_dir.mkdir(parents=True, exist_ok=True)
        
        logs_dir = self.root / "build" / "logs"
        logs_dir.mkdir(parents=True, exist_ok=True)
        
        log_file = logs_dir / "app_build.log"
        
        # Run xcodebuild
        cmd = [
            "xcodebuild",
            "-project", str(self.root / "app" / "MoreMojoStudio.xcodeproj"),
            "-scheme", "MoreMojoStudio",
            "-configuration", "Debug",
            "-destination", "platform=macOS",
            "-derivedDataPath", str(build_dir),
            "build"
        ]
        
        self.logger.info(f"Running: {' '.join(cmd)}")
        
        with open(log_file, "w") as f:
            start_time = time.time()
            process = subprocess.Popen(
                cmd,
                stdout=f,
                stderr=subprocess.STDOUT,
                text=True
            )
            
            # Wait for the build to finish with timeout
            try:
                process.wait(timeout=600)  # 10 minute timeout
                end_time = time.time()
                self.logger.info(f"Build completed in {end_time - start_time:.2f} seconds")
                
                # Check the result
                if process.returncode == 0:
                    self.logger.info("App build successful!")
                    return True
                else:
                    self.logger.error(f"App build failed with return code {process.returncode}")
                    return False
            except subprocess.TimeoutExpired:
                process.kill()
                self.logger.error("App build timed out after 10 minutes")
                return False
    
    def build_plugin(self) -> bool:
        """Build the AU and VST3 plugins."""
        self.logger.info("Building AU and VST3 plugins...")
        
        # Set up build directories
        build_dir = self.root / "build" / "plugin"
        build_dir.mkdir(parents=True, exist_ok=True)
        
        logs_dir = self.root / "build" / "logs"
        logs_dir.mkdir(parents=True, exist_ok=True)
        
        log_file = logs_dir / "plugin_build.log"
        
        # First, configure with CMake
        cmake_cmd = [
            "cmake",
            "-B", str(build_dir),
            "-S", str(self.plugin),
            f"-DMOJO_FORMATS=AU;VST3;Standalone"
        ]
        
        self.logger.info(f"Running CMake configure: {' '.join(cmake_cmd)}")
        
        with open(log_file, "w") as f:
            f.write("=== CMake Configure ===\n")
            configure_process = subprocess.Popen(
                cmake_cmd,
                stdout=f,
                stderr=subprocess.STDOUT,
                text=True
            )
            
            try:
                configure_process.wait(timeout=300)  # 5 minute timeout
                
                if configure_process.returncode != 0:
                    self.logger.error(f"Plugin CMake configure failed with return code {configure_process.returncode}")
                    return False
                
                # Now build with CMake
                f.write("\n\n=== CMake Build ===\n")
                build_cmd = [
                    "cmake",
                    "--build", str(build_dir),
                    "--config", "Debug",
                    "--parallel", str(os.cpu_count() or 4)
                ]
                
                self.logger.info(f"Running CMake build: {' '.join(build_cmd)}")
                
                build_process = subprocess.Popen(
                    build_cmd,
                    stdout=f,
                    stderr=subprocess.STDOUT,
                    text=True
                )
                
                build_process.wait(timeout=600)  # 10 minute timeout
                
                if build_process.returncode == 0:
                    self.logger.info("Plugin build successful!")
                    return True
                else:
                    self.logger.error(f"Plugin build failed with return code {build_process.returncode}")
                    return False
                
            except subprocess.TimeoutExpired:
                configure_process.kill()
                self.logger.error("Plugin build timed out")
                return False
    
    def build_all(self) -> (bool, bool):
        """Build both app and plugins."""
        self.logger.info("Building app and plugins...")
        app_result = self.build_app()
        plugin_result = self.build_plugin()
        
        if app_result and plugin_result:
            self.logger.info("All builds successful!")
        else:
            if not app_result:
                self.logger.error("App build failed")
            if not plugin_result:
                self.logger.error("Plugin build failed")
        
        return (app_result, plugin_result)
