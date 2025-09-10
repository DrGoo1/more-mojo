#!/usr/bin/env bash
set -euo pipefail

# GitHub repository owner and name
REPO="DrGoo1/more-mojo"

# GitHub personal access token (from environment or argument)
TOKEN=${GITHUB_TOKEN:-""}
if [ -z "$TOKEN" ] && [ -f "scripts/token_storage.txt" ]; then
  TOKEN=$(cat scripts/token_storage.txt)
fi
if [ -z "$TOKEN" ] && [ $# -ge 1 ]; then
  TOKEN="$1"
fi

if [ -z "$TOKEN" ]; then
  echo "ERROR: GitHub token not provided. Use GITHUB_TOKEN env var or first argument."
  exit 1
fi

# Get the latest workflow runs
echo "Fetching latest workflow runs for $REPO..."
RUNS=$(curl -s -H "Authorization: token $TOKEN" \
  "https://api.github.com/repos/$REPO/actions/runs?per_page=5")

# Check if API call was successful
if echo "$RUNS" | grep -q "API rate limit exceeded" || echo "$RUNS" | grep -q "Bad credentials"; then
  echo "ERROR: GitHub API error: $(echo "$RUNS" | grep -o '"message":"[^"]*"')"
  exit 1
fi

# Extract and display the latest workflow runs
echo ""
echo "Latest workflow runs:"
echo "===================="
echo "$RUNS" | jq -r '.workflow_runs[] | "\(.id) | \(.name) | \(.status) | \(.conclusion // "running") | \(.created_at)"' | 
  awk -F'|' '{printf "%-10s %-25s %-10s %-12s %s\n", $1, $2, $3, $4, $5}' | 
  head -n 5

# If a run ID is provided, get the details
if [ $# -ge 2 ]; then
  RUN_ID="$2"
  echo ""
  echo "Fetching details for run $RUN_ID..."
  JOBS=$(curl -s -H "Authorization: token $TOKEN" \
    "https://api.github.com/repos/$REPO/actions/runs/$RUN_ID/jobs")
  
  echo ""
  echo "Jobs for run $RUN_ID:"
  echo "===================="
  echo "$JOBS" | jq -r '.jobs[] | "\(.id) | \(.name) | \(.status) | \(.conclusion // "running") | \(.steps | length) steps"' | 
    awk -F'|' '{printf "%-10s %-25s %-10s %-12s %s\n", $1, $2, $3, $4, $5}'
  
  # If a job ID is provided, get the logs
  if [ $# -ge 3 ]; then
    JOB_ID="$3"
    echo ""
    echo "Fetching logs for job $JOB_ID..."
    LOGS_URL=$(curl -s -H "Authorization: token $TOKEN" \
      "https://api.github.com/repos/$REPO/actions/jobs/$JOB_ID" | jq -r '.logs_url')
    
    LOGS_FILE="job_${JOB_ID}_logs.txt"
    curl -s -H "Authorization: token $TOKEN" -L "$LOGS_URL" -o "$LOGS_FILE"
    
    echo "Logs saved to $LOGS_FILE"
    echo ""
    echo "Last 20 lines of logs:"
    echo "===================="
    tail -n 20 "$LOGS_FILE"
    
    # Check for common errors
    echo ""
    echo "Error summary:"
    echo "===================="
    grep -E 'error:|failed:|exception:|fatal:' "$LOGS_FILE" | tail -n 10
  fi
fi

echo ""
echo "Usage:"
echo "  $0 [TOKEN] - Show recent workflow runs"
echo "  $0 [TOKEN] [RUN_ID] - Show jobs for a specific run"
echo "  $0 [TOKEN] [RUN_ID] [JOB_ID] - Download and show logs for a specific job"
