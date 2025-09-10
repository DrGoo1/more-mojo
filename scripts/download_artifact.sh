#!/usr/bin/env bash
# Script to download GitHub artifact using stored token

# Load token from storage
TOKEN_FILE="/Volumes/MP 1/MoreMojo/ChatGPT/scripts/token_storage.txt"
if [ ! -f "$TOKEN_FILE" ]; then
  echo "Error: Token file not found at $TOKEN_FILE"
  exit 1
fi

# Read token from file
TOKEN=$(grep 'GITHUB_TOKEN=' "$TOKEN_FILE" | cut -d'=' -f2)
if [ -z "$TOKEN" ]; then
  echo "Error: No token found in $TOKEN_FILE"
  echo "Please edit $TOKEN_FILE and add your GitHub token after GITHUB_TOKEN="
  exit 1
fi

# Set up GitHub CLI with token
export GH_TOKEN="$TOKEN"

# Authenticate and test
echo "Testing authentication..."
gh auth status || {
  echo "Authentication failed. Please check your token."
  exit 1
}

# Get latest workflow run ID
echo "Finding latest workflow run..."
RUN_ID=$(gh run list -R DrGoo1/more-mojo --workflow "Build macOS App" --branch main -L 1 --json databaseId --jq '.[0].databaseId')

if [ -z "$RUN_ID" ]; then
  echo "Error: Could not find workflow run ID"
  exit 1
fi

echo "Found workflow run ID: $RUN_ID"

# Create download directory
DOWNLOAD_DIR="/Volumes/MP 1/MoreMojo/ChatGPT/downloads"
mkdir -p "$DOWNLOAD_DIR"

# Download the artifact
echo "Downloading artifact..."
gh run download -R DrGoo1/more-mojo "$RUN_ID" -n "MoreMojo-App" -D "$DOWNLOAD_DIR"

echo "Done! Check $DOWNLOAD_DIR for downloaded artifacts"
