#!/usr/bin/env python3
"""Base agent class that all agents should inherit from."""

import abc
import logging
from pathlib import Path
from typing import Dict, List, Optional, Any

# Set up logging
logging.basicConfig(level=logging.INFO, format='%(name)s - %(levelname)s - %(message)s')

class BaseAgent(abc.ABC):
    """Base agent class that all agents should inherit from."""

    def __init__(self, root: Path):
        """Initialize the agent.
        
        Args:
            root: Root directory of the project
        """
        self.root = root
        self.logger = logging.getLogger(self.__class__.__name__)
        self.src = root / "app" / "Sources"
        self.plugin = root / "plugin"
    
    @classmethod
    @abc.abstractmethod
    def name(cls) -> str:
        """Return the name of the agent."""
        pass
    
    @classmethod
    @abc.abstractmethod
    def description(cls) -> str:
        """Return a description of what the agent does."""
        pass
    
    @abc.abstractmethod
    def wants(self, app_logs: str, plugin_logs: str) -> bool:
        """Determine if the agent wants to run based on log content.
        
        Args:
            app_logs: Log content from app build
            plugin_logs: Log content from plugin build
            
        Returns:
            True if the agent wants to run, False otherwise
        """
        pass
    
    @abc.abstractmethod
    def run(self) -> bool:
        """Run the agent to fix detected issues.
        
        Returns:
            True if changes were made, False otherwise
        """
        pass
    
    def write_file(self, path: Path, content: str) -> bool:
        """Write content to a file if it's different from current content.
        
        Args:
            path: Path to file
            content: Content to write
            
        Returns:
            True if file was written, False otherwise
        """
        path.parent.mkdir(parents=True, exist_ok=True)
        old = path.read_text(errors="ignore") if path.exists() else ""
        if old == content:
            return False
        path.write_text(content)
        self.logger.info(f"Wrote {path}")
        return True
