# GitHub Artifacts Download Guide

This guide explains how to download build artifacts using the GitHub CLI.

## Prerequisites

1. Install the GitHub CLI:
   ```bash
   brew install gh
   ```

2. Authenticate with GitHub:
   ```bash
   # Use a token with repo and actions:read permissions
   export GH_TOKEN="your_token_here"
   gh auth login --with-token <<< "$GH_TOKEN"
   ```

## Download Artifacts

### Option 1: Download from specific run

```bash
# Get artifacts from a specific run
gh run download 17590206072 -n "MoreMojo-App" -D ./downloads
```

### Option 2: Download latest artifacts

```bash
# List the latest workflow runs
gh run list --workflow "Build macOS App" --branch main -L 1

# Extract the run ID and download artifacts
RUN_ID=$(gh run list --workflow "Build macOS App" --branch main -L 1 --json databaseId --jq '.[0].databaseId')
gh run download $RUN_ID -n "MoreMojo-App" -D ./downloads
```

### Option 3: One-liner for latest artifacts

```bash
# Download the latest app artifact
gh run download $(gh run list --workflow "Build macOS App" --branch main -L 1 --json databaseId --jq '.[0].databaseId') -n "MoreMojo-App" -D ./downloads

# Download the latest plugin artifacts
gh run download $(gh run list --workflow "Build Plugin (macOS)" --branch main -L 1 --json databaseId --jq '.[0].databaseId') -n "MoreMojo-Plugins" -D ./downloads
```

## For CI Systems (Windsurf)

Add this to your CI system:

```bash
# Setup GitHub CLI with token
export GH_TOKEN="$WINDSURF_TOKEN"
gh auth login --with-token <<< "$GH_TOKEN"

# Get latest successful run ID
RUN_ID=$(gh run list --workflow "Build macOS App" --branch main --status completed -L 1 --json databaseId --jq '.[0].databaseId')

# Download artifacts
gh run download $RUN_ID -n "MoreMojo-App" -D ./downloads

# Extract and use the app
unzip -o ./downloads/MoreMojo-App/*.zip -d ./app-extracted
```

## Troubleshooting

1. **Authentication Issues**:
   - Ensure your token has `repo` and `actions:read` permissions
   - For private repos, make sure your token has access

2. **No Artifacts Found**:
   - Check if the workflow run completed successfully
   - Verify the artifact name is correct (`MoreMojo-App` or `MoreMojo-Plugins`)

3. **Permission Denied**:
   - Confirm you have access rights to the repository
   - Verify you're authenticated with the correct GitHub account
