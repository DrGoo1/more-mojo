"""
Agent modules for the MoreMojo swarm automation system.
This package contains specialized agents that can detect and fix specific issues.
"""

# Import all agents to make them available through the package
from .base_agent import BaseAgent
from .project_agent import ProjectAgent
from .scheme_agent import SchemeAgent
from .swift_agent import SwiftAgent
from .cmake_agent import CMakeAgent
from .warmth_agent import WarmthAgent
from .import_agent import ImportAgent
from .analyzer_agent import AnalyzerAgent
from .struct_agent import StructAgent

# Export the registry for easy access
from .registry import agent_registry
