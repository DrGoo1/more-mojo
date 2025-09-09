#!/usr/bin/env bash
# Script to authenticate with GitHub and trigger workflows in sequence

set -e  # Exit on error

# Configuration
REPO="DrGoo1/more-mojo"
TOKEN_FILE="/Volumes/MP 1/MoreMojo/ChatGPT/scripts/token_storage.txt"
TIMEOUT=600  # Max time to wait for a workflow in seconds (10 minutes)

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if token file exists
if [ ! -f "$TOKEN_FILE" ]; then
    echo -e "${RED}Error: Token file not found at $TOKEN_FILE${NC}"
    echo "Please run: echo 'GITHUB_TOKEN=your_token_here' > $TOKEN_FILE"
    exit 1
fi

# Read token from file
TOKEN=$(grep 'GITHUB_TOKEN=' "$TOKEN_FILE" | cut -d'=' -f2)
if [ -z "$TOKEN" ]; then
    echo -e "${RED}Error: No token found in $TOKEN_FILE${NC}"
    echo "Please edit $TOKEN_FILE and add your GitHub token after GITHUB_TOKEN="
    exit 1
fi

# Export token for GitHub CLI
export GH_TOKEN="$TOKEN"

# Skip formal authentication and just use the token directly with each command
echo -e "${BLUE}Using token-based authentication for all commands...${NC}"

# Test token by making a simple API call
echo -e "${YELLOW}Testing token validity...${NC}"
if ! curl -s -o /dev/null -w "%{http_code}" -H "Authorization: token $GH_TOKEN" "https://api.github.com/repos/$REPO" | grep -q "^2"; then
    echo -e "${RED}Invalid token or repository not accessible. Please check your token.${NC}"
    exit 1
fi
echo -e "${GREEN}Token valid!${NC}"

# Function to trigger a workflow and wait for it to complete
trigger_workflow() {
    local workflow_name=$1
    local workflow_file=$2
    shift 2
    local params=("$@")
    
    echo -e "\n${BLUE}===============================================${NC}"
    echo -e "${BLUE}Triggering $workflow_name workflow${NC}"
    echo -e "${BLUE}===============================================${NC}"
    
    # Build command - add --header for direct token usage
    local cmd="gh workflow run $workflow_file -R $REPO --header 'Authorization: token $GH_TOKEN'"
    
    # Add parameters if they exist
    for param in "${params[@]}"; do
        cmd="$cmd $param"
    done
    
    # Run the command
    echo -e "${YELLOW}Running: $cmd${NC}"
    if ! eval "$cmd"; then
        echo -e "${RED}Failed to trigger $workflow_name workflow${NC}"
        return 1
    fi
    
    # Get the run ID of the triggered workflow
    echo -e "${YELLOW}Waiting for workflow to start...${NC}"
    sleep 5  # Wait for workflow to appear in the list
    
    local run_id=""
    local elapsed=0
    
    # Wait for the workflow to start and get its ID
    while [ -z "$run_id" ] && [ $elapsed -lt 60 ]; do
        run_id=$(gh run list --workflow "$workflow_file" -R "$REPO" --limit 1 --json databaseId --jq '.[0].databaseId' 2>/dev/null || echo "")
        if [ -z "$run_id" ]; then
            sleep 5
            elapsed=$((elapsed + 5))
        fi
    done
    
    if [ -z "$run_id" ]; then
        echo -e "${RED}Could not find the triggered workflow run${NC}"
        return 1
    fi
    
    echo -e "${GREEN}Workflow started with Run ID: $run_id${NC}"
    
    # Poll for completion
    local status=""
    local conclusion=""
    elapsed=0
    
    while [ "$status" != "completed" ] && [ $elapsed -lt $TIMEOUT ]; do
        result=$(gh run view "$run_id" -R "$REPO" --json status,conclusion 2>/dev/null || echo '{"status":"unknown"}')
        status=$(echo "$result" | jq -r '.status')
        conclusion=$(echo "$result" | jq -r '.conclusion')
        
        echo -e "${YELLOW}Status: $status${NC}"
        if [ "$status" == "completed" ]; then
            break
        fi
        
        sleep 10
        elapsed=$((elapsed + 10))
    done
    
    # Check result
    if [ "$status" != "completed" ]; then
        echo -e "${RED}Workflow did not complete within the timeout period${NC}"
        return 1
    elif [ "$conclusion" != "success" ]; then
        echo -e "${RED}Workflow completed with status: $conclusion${NC}"
        return 1
    else
        echo -e "${GREEN}Workflow completed successfully!${NC}"
        return 0
    fi
}

# Main sequence
echo -e "${BLUE}Starting phased workflow triggers${NC}"

# Phase 1.5: Build App with Steal Mojo Integration
echo -e "\n${GREEN}=== PHASE 1.5: Building App with Steal Mojo ===${NC}"
if ! trigger_workflow "Phase 1.5 - Steal That Mojo (Fixed)" "phase_1_5_fix.yml"; then
    echo -e "${RED}Phase 1.5 failed. Stopping sequence.${NC}"
    exit 1
fi

# Phase 2: Plugin Parity
echo -e "\n${GREEN}=== PHASE 2: Plugin Parity ===${NC}"
if ! trigger_workflow "Build Plugin Phase 2" "build_plugin_phase2.yml"; then
    echo -e "${RED}Phase 2 failed. Stopping sequence.${NC}"
    exit 1
fi

# Phase 3: Evaluator (only if the evaluator directory exists)
if [ -d "/Volumes/MP 1/MoreMojo/ChatGPT/tools/evaluator_cli" ]; then
    echo -e "\n${GREEN}=== PHASE 3: Evaluator ===${NC}"
    if ! trigger_workflow "Evaluator" "evaluate.yml" "-f type=vocal -f plugin=\"Apple: AUDistortion\""; then
        echo -e "${RED}Phase 3 failed. Continuing to next phase.${NC}"
    fi
else
    echo -e "\n${YELLOW}Skipping Phase 3: Evaluator directory not found${NC}"
fi

# Phase 4: Model Smoke Test
echo -e "\n${GREEN}=== PHASE 4: Model Smoke Test ===${NC}"
if ! trigger_workflow "AI Model Smoke Test" "model_smoketest.yml"; then
    echo -e "${RED}Phase 4 failed.${NC}"
fi

echo -e "\n${GREEN}Workflow sequence complete!${NC}"
