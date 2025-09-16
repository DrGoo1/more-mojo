#!/usr/bin/env python3
"""Scheme agent to fix missing or unshared schemes in Xcode projects."""

import re
import subprocess
import shutil
from pathlib import Path

from .base_agent import BaseAgent
from .registry import register_agent

@register_agent
class SchemeAgent(BaseAgent):
    """Fix missing/unshared scheme by regenerating the project with XcodeGen."""
    
    PATTERNS = [
        r"Scheme .+ is not currently configured for the build action",
        r"No shared schemes found",
        r"xcodebuild: error:",
    ]
    
    @classmethod
    def name(cls) -> str:
        return "SchemeAgent"
    
    @classmethod
    def description(cls) -> str:
        return "Fixes missing or unshared schemes in Xcode projects"
    
    def wants(self, app_logs: str, plugin_logs: str) -> bool:
        """Determine if the agent wants to run based on log content."""
        return any(re.search(p, app_logs, flags=re.IGNORECASE) for p in self.PATTERNS)
    
    def run(self) -> bool:
        """Fix missing schemes by regenerating with XcodeGen or creating a default scheme."""
        self.logger.info("Ensuring shared scheme via XcodeGen (if project.yml exists)")
        
        # Install xcodegen if needed
        try:
            subprocess.run("which xcodegen", shell=True, check=True, capture_output=True)
        except subprocess.CalledProcessError:
            self.logger.info("Installing XcodeGen...")
            subprocess.run("brew update || true", shell=True, check=False)
            subprocess.run("brew install xcodegen", shell=True, check=False)
        
        # Check if project.yml exists
        projy = self.root / "app" / "project.yml"
        if projy.exists():
            # Regenerate project with XcodeGen
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
        
        # If XcodeGen not available, try to create a default shared scheme
        proj_dir = self.root / "app" / "MoreMojoStudio.xcodeproj"
        schemes_dir = proj_dir / "xcshareddata" / "xcschemes"
        
        if not proj_dir.exists():
            self.logger.warning("Xcode project not found, cannot create scheme")
            return False
        
        # Create shared schemes directory if it doesn't exist
        schemes_dir.mkdir(parents=True, exist_ok=True)
        
        # Create a minimal shared scheme file
        scheme_content = """<?xml version="1.0" encoding="UTF-8"?>
<Scheme
   LastUpgradeVersion = "1420"
   version = "1.3">
   <BuildAction
      parallelizeBuildables = "YES"
      buildImplicitDependencies = "YES">
      <BuildActionEntries>
         <BuildActionEntry
            buildForTesting = "YES"
            buildForRunning = "YES"
            buildForProfiling = "YES"
            buildForArchiving = "YES"
            buildForAnalyzing = "YES">
            <BuildableReference
               BuildableIdentifier = "primary"
               BlueprintIdentifier = "APP_TARGET_ID"
               BuildableName = "MoreMojoStudio.app"
               BlueprintName = "MoreMojoStudio"
               ReferencedContainer = "container:MoreMojoStudio.xcodeproj">
            </BuildableReference>
         </BuildActionEntry>
      </BuildActionEntries>
   </BuildAction>
   <TestAction
      buildConfiguration = "Debug"
      selectedDebuggerIdentifier = "Xcode.DebuggerFoundation.Debugger.LLDB"
      selectedLauncherIdentifier = "Xcode.DebuggerFoundation.Launcher.LLDB"
      shouldUseLaunchSchemeArgsEnv = "YES">
      <Testables>
      </Testables>
   </TestAction>
   <LaunchAction
      buildConfiguration = "Debug"
      selectedDebuggerIdentifier = "Xcode.DebuggerFoundation.Debugger.LLDB"
      selectedLauncherIdentifier = "Xcode.DebuggerFoundation.Launcher.LLDB"
      launchStyle = "0"
      useCustomWorkingDirectory = "NO"
      ignoresPersistentStateOnLaunch = "NO"
      debugDocumentVersioning = "YES"
      debugServiceExtension = "internal"
      allowLocationSimulation = "YES">
      <BuildableProductRunnable
         runnableDebuggingMode = "0">
         <BuildableReference
            BuildableIdentifier = "primary"
            BlueprintIdentifier = "APP_TARGET_ID"
            BuildableName = "MoreMojoStudio.app"
            BlueprintName = "MoreMojoStudio"
            ReferencedContainer = "container:MoreMojoStudio.xcodeproj">
         </BuildableReference>
      </BuildableProductRunnable>
   </LaunchAction>
   <ProfileAction
      buildConfiguration = "Release"
      shouldUseLaunchSchemeArgsEnv = "YES"
      savedToolIdentifier = ""
      useCustomWorkingDirectory = "NO"
      debugDocumentVersioning = "YES">
      <BuildableProductRunnable
         runnableDebuggingMode = "0">
         <BuildableReference
            BuildableIdentifier = "primary"
            BlueprintIdentifier = "APP_TARGET_ID"
            BuildableName = "MoreMojoStudio.app"
            BlueprintName = "MoreMojoStudio"
            ReferencedContainer = "container:MoreMojoStudio.xcodeproj">
         </BuildableReference>
      </BuildableProductRunnable>
   </ProfileAction>
   <AnalyzeAction
      buildConfiguration = "Debug">
   </AnalyzeAction>
   <ArchiveAction
      buildConfiguration = "Release"
      revealArchiveInOrganizer = "YES">
   </ArchiveAction>
</Scheme>
"""
        
        # Try to extract target ID from project.pbxproj
        proj_file = proj_dir / "project.pbxproj"
        target_id = "UNKNOWN_TARGET_ID"
        
        if proj_file.exists():
            content = proj_file.read_text()
            match = re.search(r'([A-F0-9]{24}) \/\* MoreMojoStudio \*\/', content)
            if match:
                target_id = match.group(1)
        
        # Replace placeholder with actual target ID
        scheme_content = scheme_content.replace("APP_TARGET_ID", target_id)
        
        # Write the scheme file
        scheme_file = schemes_dir / "MoreMojoStudio.xcscheme"
        changed = self.write_file(scheme_file, scheme_content)
        
        if changed:
            self.logger.info(f"Created shared scheme at {scheme_file}")
            return True
        else:
            self.logger.info("Shared scheme already exists")
            return False
