#!/usr/bin/env python3
"""Import agent to fix missing or incorrect import statements."""

import re
from pathlib import Path

from .base_agent import BaseAgent
from .registry import register_agent

@register_agent
class ImportAgent(BaseAgent):
    """Fix issues with imports in Swift files."""
    
    KEYS = [
        "No such module",
        "Unable to find module dependency",
        "use of undeclared type", 
        "unknown type name",
        "Cannot find",
        "@_exported import",
    ]
    
    @classmethod
    def name(cls) -> str:
        return "ImportAgent"
    
    @classmethod
    def description(cls) -> str:
        return "Fixes issues with imports in Swift files"
    
    def wants(self, app_logs: str, plugin_logs: str) -> bool:
        """Determine if the agent wants to run based on log content."""
        return any(k in app_logs for k in self.KEYS)
    
    def run(self) -> bool:
        """Fix import issues."""
        changed = False
        
        # Map of files to required imports
        import_map = {
            "StealMojoPanel_SwiftOnly.swift": ["SwiftUI", "AVFoundation"],
            "MoreMojoSimpleView.swift": ["SwiftUI"],
            "MoreMojoContainer.swift": ["SwiftUI", "Combine"],
            "AudioEngine.swift": ["AVFoundation", "Foundation", "Combine"],
        }
        
        # Check all Swift files for import issues
        for swift_file in self.src.glob("*.swift"):
            content = swift_file.read_text(errors="ignore")
            new_content = content
            
            # Remove @_exported imports which can cause issues
            if "@_exported import" in new_content:
                self.logger.info(f"Removing @_exported imports in {swift_file.name}")
                new_content = re.sub(r'@_exported import [^\n]+\n', '', new_content)
            
            # Add required imports if missing
            if swift_file.name in import_map:
                required_imports = import_map[swift_file.name]
                for imp in required_imports:
                    if f"import {imp}" not in new_content:
                        self.logger.info(f"Adding import {imp} to {swift_file.name}")
                        new_content = f"import {imp}\n{new_content}"
            
            # Ensure all import statements are at the top of the file
            import_lines = re.findall(r'^import [^\n]+$', new_content, re.MULTILINE)
            if import_lines:
                # Remove all imports and add them at the top
                for imp in import_lines:
                    if imp in new_content[20:]:  # Skip the very top to avoid duplicate removal
                        new_content = new_content.replace(imp + "\n", "")
                
                # Compile all unique imports
                unique_imports = set(import_lines)
                imports_text = "\n".join(sorted(unique_imports)) + "\n\n"
                
                # Add back at the top, preserving any comments or file headers
                if new_content.startswith("//"):
                    # Find the end of the comment block
                    comment_end = new_content.find("\n\n")
                    if comment_end > 0:
                        new_content = new_content[:comment_end+2] + imports_text + new_content[comment_end+2:]
                    else:
                        new_content = imports_text + new_content
                else:
                    new_content = imports_text + new_content.lstrip()
            
            if new_content != content:
                self.write_file(swift_file, new_content)
                self.logger.info(f"Updated imports in {swift_file.name}")
                changed = True
        
        return changed
