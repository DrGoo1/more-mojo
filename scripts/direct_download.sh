#!/bin/bash
# Direct workflow artifact download script

# Get the latest workflow run ID for the specified workflow
get_latest_run_id() {
  local owner="$1"
  local repo="$2"
  local workflow="$3"
  local token="$4"
  
  curl -s -H "Authorization: token $token" \
    "https://api.github.com/repos/$owner/$repo/actions/workflows/$workflow/runs?per_page=1" | \
    grep -o '"id":[0-9]*' | head -1 | cut -d':' -f2
}

# Main function
main() {
  if [ $# -lt 1 ]; then
    echo "Usage: $0 <token> [workflow_name]"
    echo "Example: $0 ghp_abc123 build_app.yml"
    exit 1
  fi

  TOKEN="$1"
  WORKFLOW="${2:-build_app.yml}"
  OWNER="DrGoo1"
  REPO="more-mojo"
  
  echo "Finding latest run ID for workflow $WORKFLOW..."
  RUN_ID=$(get_latest_run_id "$OWNER" "$REPO" "$WORKFLOW" "$TOKEN")
  
  if [ -z "$RUN_ID" ]; then
    echo "Error: Could not find a workflow run. Check your token and workflow name."
    exit 1
  fi
  
  echo "Found run ID: $RUN_ID"
  
  # Create direct download URL
  ARTIFACTS_URL="https://github.com/$OWNER/$REPO/actions/runs/$RUN_ID/artifacts"
  
  echo "✅ Your artifacts are available at:"
  echo "$ARTIFACTS_URL"
  echo
  echo "Open this URL in your browser to download artifacts directly."
  echo "You will need to be logged into GitHub with an account that has access to the repository."
}

main "$@"
