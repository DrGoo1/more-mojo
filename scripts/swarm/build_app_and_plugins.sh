#!/usr/bin/env bash
# build_app_and_plugins.sh - Build the app and plugins using the agent system

set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

# Colors for better output
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
BLUE="\033[0;34m"
RED="\033[0;31m"
NC="\033[0m" # No Color

function log_info() { echo -e "${BLUE}[INFO]${NC} $*"; }
function log_warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
function log_error() { echo -e "${RED}[ERROR]${NC} $*"; }
function log_success() { echo -e "${GREEN}[SUCCESS]${NC} $*"; }

# Check if Python is available
if ! command -v python3 &> /dev/null; then
    log_error "Python 3 is required but not found. Please install it."
    exit 1
fi

# Create directories
mkdir -p "$ROOT/build/logs"

# Run the agent_hub_v2.py script with build option
log_info "Starting agentic build process for app and plugins..."

# First, run agents to fix any issues
python3 "$ROOT/scripts/swarm/agent_hub_v2.py" --fix

# If there were changes, ask if we should commit them
if [ "$(git -C "$ROOT" status --porcelain | wc -l)" -ne 0 ]; then
    log_info "Changes were detected. Would you like to commit them? (y/n)"
    read -r response
    if [[ "$response" =~ ^[Yy] ]]; then
        log_info "Committing changes..."
        git -C "$ROOT" add -A
        git -C "$ROOT" commit -m "Swarm: Fixed build issues detected by agents"
        log_success "Changes committed."
    else
        log_info "Changes not committed. Proceeding with build."
    fi
fi

# Now build the app and plugins
log_info "Building app and plugins..."
python3 "$ROOT/scripts/swarm/agent_hub_v2.py" --all

# Check build results
if [ $? -eq 0 ]; then
    log_success "Build completed successfully!"
    
    # Show app and plugin locations
    APP_PATH="$ROOT/build/app/Build/Products/Debug/MoreMojoStudio.app"
    PLUGIN_PATH="$ROOT/build/plugin"
    
    log_info "App location: $APP_PATH"
    log_info "Plugins location: $PLUGIN_PATH"
    
    # Open app location in Finder if it exists
    if [ -d "$APP_PATH" ]; then
        log_info "Opening app location in Finder..."
        open "$(dirname "$APP_PATH")"
    fi
else
    log_error "Build failed. Check logs for details."
    log_info "Log location: $ROOT/build/logs"
    exit 1
fi
