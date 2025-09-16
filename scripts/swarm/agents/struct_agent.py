#!/usr/bin/env python3
"""Struct agent to fix struct and initializer issues."""

import re
from pathlib import Path

from .base_agent import BaseAgent
from .registry import register_agent

@register_agent
class StructAgent(BaseAgent):
    """Fix issues with struct initializers and field references."""
    
    KEYS = [
        "no member named",
        "cannot infer contextual base in reference to member 'init'",
        "argument passed to call that takes no arguments",
        "missing argument",
        "initializer requires",
        "does not match"
    ]
    
    @classmethod
    def name(cls) -> str:
        return "StructAgent"
    
    @classmethod
    def description(cls) -> str:
        return "Fixes issues with struct initializers and field references"
    
    def wants(self, app_logs: str, plugin_logs: str) -> bool:
        """Determine if the agent wants to run based on log content."""
        return any(k in app_logs for k in self.KEYS)
    
    def run(self) -> bool:
        """Fix struct issues."""
        changed = False
        
        # Fix issues in Swift files
        for swift_file in self.src.glob("*.swift"):
            content = swift_file.read_text(errors="ignore")
            new_content = content
            
            # Fix initializers with wrong argument count
            init_patterns = [
                # Missing argument labels or wrong number of arguments
                (r'\.init\(([^)]*)\)', lambda m: self._fix_initializer(m)),
                # Initializing with wrong type
                (r'MojoEQBand\((lo|hi|gain_dB): ([^,]+), (lo|hi|gain_dB): ([^,]+), (lo|hi|gain_dB): ([^)]+)\)', 
                 lambda m: self._fix_eq_band_initializer(m)),
            ]
            
            for pattern, replace_func in init_patterns:
                new_content = re.sub(pattern, replace_func, new_content)
            
            # Fix specific struct instantiation patterns
            if "MojoRecommendation(" in new_content:
                rec_pattern = r'MojoRecommendation\([^)]+\)'
                for match in re.finditer(rec_pattern, new_content):
                    if "interpMode:" not in match.group(0):
                        replacement = self._fix_recommendation_initializer(match.group(0))
                        new_content = new_content.replace(match.group(0), replacement)
            
            if new_content != content:
                self.write_file(swift_file, new_content)
                self.logger.info(f"Updated struct initializers in {swift_file.name}")
                changed = True
        
        return changed
    
    def _fix_initializer(self, match):
        """Fix initializers with wrong argument count."""
        args = match.group(1).strip()
        
        # If it's an empty initializer, keep it as is
        if not args:
            return ".init()"
            
        # Check for various patterns that need fixing
        if ":" not in args and "," in args:
            # Missing argument labels - assume we're dealing with MojoEQBand
            parts = [p.strip() for p in args.split(",")]
            if len(parts) == 3:
                return f".init(lo: {parts[0]}, hi: {parts[1]}, gain_dB: {parts[2]})"
        
        # Default case: keep as is
        return f".init({args})"
    
    def _fix_eq_band_initializer(self, match):
        """Fix MojoEQBand initializers."""
        # Extract all parts
        parts = {}
        for i in range(1, 7, 2):
            label = match.group(i)
            value = match.group(i+1)
            parts[label] = value
        
        # Ensure all required fields are present
        for field in ["lo", "hi", "gain_dB"]:
            if field not in parts:
                parts[field] = "0.0"
                
        return f"MojoEQBand(lo: {parts['lo']}, hi: {parts['hi']}, gain_dB: {parts['gain_dB']})"
    
    def _fix_recommendation_initializer(self, init_text):
        """Fix MojoRecommendation initializers."""
        # Extract existing arguments
        args_match = re.search(r'MojoRecommendation\(([^)]+)\)', init_text)
        if not args_match:
            return init_text
            
        args_text = args_match.group(1)
        args_dict = {}
        
        # Parse arguments
        if ":" in args_text:
            # Named arguments
            for arg in args_text.split(","):
                if ":" in arg:
                    parts = arg.split(":", 1)
                    name = parts[0].strip()
                    value = parts[1].strip()
                    args_dict[name] = value
        else:
            # Positional arguments
            pos_args = [arg.strip() for arg in args_text.split(",")]
            arg_names = ["interpMode", "drive", "saturation", "character", "presence", "mix", "output"]
            for i, arg_name in enumerate(arg_names):
                if i < len(pos_args):
                    args_dict[arg_name] = pos_args[i]
        
        # Ensure all required fields are present
        required_fields = {
            "interpMode": '"liveHB4x"',
            "drive": "0.5",
            "saturation": "0.5", 
            "character": "0.5",
            "presence": "0.5",
            "mix": "1.0",
            "output": "0.0"
        }
        
        for field, default in required_fields.items():
            if field not in args_dict:
                args_dict[field] = default
        
        # Build the new initializer
        new_args = []
        for field in ["interpMode", "drive", "saturation", "character", "presence", "mix", "output"]:
            new_args.append(f"{field}: {args_dict[field]}")
            
        return f"MojoRecommendation({', '.join(new_args)})"
