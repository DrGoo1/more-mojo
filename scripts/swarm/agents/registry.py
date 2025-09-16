#!/usr/bin/env python3
"""Registry for all swarm agents."""

import logging
from pathlib import Path
from typing import Dict, List, Type, Any

# Set up logging
logging.basicConfig(level=logging.INFO, format='%(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger("AgentRegistry")

class AgentRegistry:
    """Registry for all swarm agents."""
    
    def __init__(self):
        """Initialize the agent registry."""
        self._agents = {}
        
    def register(self, agent_class):
        """Register an agent with the registry.
        
        Args:
            agent_class: Agent class to register
            
        Returns:
            The agent class for chaining
        """
        agent_name = agent_class.name()
        if agent_name in self._agents:
            logger.warning(f"Agent {agent_name} is already registered, overwriting")
        self._agents[agent_name] = agent_class
        return agent_class
        
    def get_agent(self, name: str) -> Any:
        """Get an agent by name.
        
        Args:
            name: Name of the agent
            
        Returns:
            Agent class or None if not found
        """
        return self._agents.get(name)
        
    def get_all_agents(self) -> Dict[str, Any]:
        """Get all registered agents.
        
        Returns:
            Dictionary of agent name to agent class
        """
        return self._agents.copy()
        
    def instantiate_agents(self, root: Path) -> List[Any]:
        """Instantiate all registered agents.
        
        Args:
            root: Root directory of the project
            
        Returns:
            List of instantiated agents
        """
        agents = []
        for agent_class in self._agents.values():
            try:
                agents.append(agent_class(root))
            except Exception as e:
                logger.error(f"Failed to instantiate agent {agent_class.name()}: {e}")
        return agents


# Create global agent registry
agent_registry = AgentRegistry()

def register_agent(cls):
    """Decorator to register an agent with the registry.
    
    Args:
        cls: Agent class to register
        
    Returns:
        The agent class
    """
    return agent_registry.register(cls)
