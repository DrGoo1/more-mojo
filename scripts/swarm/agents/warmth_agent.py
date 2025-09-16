#!/usr/bin/env python3
"""Warmth agent to fix issues with the warmth parameter."""

import re
from pathlib import Path

from .base_agent import BaseAgent
from .registry import register_agent

@register_agent
class WarmthAgent(BaseAgent):
    """Fix issues related to warmth property in ProcessorParams."""
    
    KEYS = [
        "value of type 'ProcessorParams' has no member 'warmth'",
        "has no member named 'warmth'", 
        "warmth",
        "No exact matches in call to initializer",
    ]
    
    @classmethod
    def name(cls) -> str:
        return "WarmthAgent"
    
    @classmethod
    def description(cls) -> str:
        return "Ensures the warmth property is properly added to ProcessorParams"
    
    def wants(self, app_logs: str, plugin_logs: str) -> bool:
        """Determine if the agent wants to run based on log content."""
        return any(k in app_logs for k in self.KEYS)
    
    def run(self) -> bool:
        """Fix warmth property issues."""
        changed = False
        
        # Check SharedTypes.swift to ensure warmth is properly defined
        shared_types = self.src / "SharedTypes.swift"
        if shared_types.exists():
            content = shared_types.read_text(errors="ignore")
            if "public var warmth: Float" not in content:
                self.logger.info("Adding warmth property to ProcessorParams in SharedTypes.swift")
                new_content = re.sub(
                    r'(public var presence: Float = 0\.50\n)(\s+public var mix: Float = 1\.00)',
                    r'\1    public var warmth: Float = 0.60\n\2',
                    content
                )
                if new_content != content:
                    self.write_file(shared_types, new_content)
                    changed = True
        
        # Check for warmth usage in files and update as necessary
        for swift_file in self.src.glob("*.swift"):
            if swift_file.name in ["SharedTypes.swift", "ProcessorParams+Ext.swift"]:
                continue
                
            content = swift_file.read_text(errors="ignore")
            new_content = content
            
            # Look for warmth references with no definition
            if "warmth" in new_content and "params.warmth" not in new_content:
                # Add warmth property to dictionary conversions
                new_content = re.sub(
                    r'(\s+"presence": [^,\n]+,)(\s+"mix": [^,\n]+)',
                    r'\1\n        "warmth": 0.6,\2',
                    new_content
                )
                
                # Add warmth to parameter assignments
                new_content = re.sub(
                    r'(params\.presence = [^;\n]+;)(\s+params\.mix = [^;\n]+)',
                    r'\1\n        params.warmth = 0.6;\2',
                    new_content
                )
            
            if new_content != content:
                self.write_file(swift_file, new_content)
                self.logger.info(f"Updated {swift_file.name} with warmth property handling")
                changed = True
        
        return changed
