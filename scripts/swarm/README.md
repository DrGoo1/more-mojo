# MoreMojo Swarm System

The Swarm System is an automated CI/CD fix system that detects and repairs common build issues in the MoreMojo project.

## Components

### 1. Agent Hub (`agent_hub.py`)

The central coordinator that reads build logs and activates specialized agents to fix specific issues:

- **ProjectAgent**: Fixes Xcode project format issues by regenerating with XcodeGen
- **SchemeAgent**: Handles missing/unshared scheme issues
- **SwiftAgent**: Fixes Swift code issues including type duplication and syntax problems
- **CMakeAgent**: Fixes CMake configuration issues in plugin builds

### 2. Local Testing (`run_local.sh`)

Test the Swarm system locally before pushing changes:

```bash
# Download and process latest CI artifacts
./scripts/swarm/run_local.sh --download

# Test with specific failure types
./scripts/swarm/run_local.sh --test-failure swift
./scripts/swarm/run_local.sh --test-failure scheme
./scripts/swarm/run_local.sh --test-failure cmake

# Clean up after testing
./scripts/swarm/run_local.sh --cleanup swift

# Use local logs instead of downloading
./scripts/swarm/run_local.sh --local-logs
```

### 3. GitHub Actions Integration

The Swarm system is integrated into the CI/CD pipeline via the `autofix` job in `.github/workflows/build_with_logs.yml`. It automatically:

1. Triggers when app or plugin builds fail
2. Downloads build logs and artifacts
3. Runs the agent hub to detect and fix issues
4. Creates a PR with fixes if changes are made

## How to Add New Agents

To add a new agent:

1. Add a new agent class to `agent_hub.py` with:
   - `wants(logs)` method that returns True if this agent should activate
   - `run()` method that applies fixes
2. Add the agent to the `decisions` dictionary in the `main()` function
3. Add test cases to `run_local.sh`'s `setup_test_failure()` function

## Common Issues and Solutions

| Issue | Agent | Solution |
|-------|-------|----------|
| Future Xcode format | ProjectAgent | Regenerate using XcodeGen |
| Missing schemes | SchemeAgent | Regenerate using XcodeGen |
| Swift type duplication | SwiftAgent | Unify types in SharedTypes.swift |
| CMake generator issues | CMakeAgent | Fix CMakeLists.txt |
