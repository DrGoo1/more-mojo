# GitHub Artifacts Download Instructions

I've created a Python script to download artifacts directly from GitHub Actions without needing to navigate the web interface.

## Step 1: Create a GitHub Personal Access Token

### Classic Token (Recommended)

1. Go to https://github.com/settings/tokens
2. Click "Generate new token (classic)" 
3. Give it a name like "MoreMojo Artifact Downloader"
4. Select the following scopes:
   - `repo` (Full control of private repositories)
   - `workflow` (Update GitHub Action workflows)
   - `read:packages` (Download packages from GitHub Package Registry)
5. Click "Generate token"
6. **IMPORTANT**: Copy the generated token - you won't be able to see it again!

> **NOTE**: Fine-grained PATs (that start with `github_pat_`) often have permission issues with artifact downloads. A classic token is recommended for this script.

## Step 2: Run the Download Script

```bash
# Navigate to your project directory
cd "/Volumes/MP 1/MoreMojo/ChatGPT"

# Run the script with your token
python3 scripts/download_artifacts.py YOUR_TOKEN_HERE

# If you want to download artifacts from a specific workflow:
python3 scripts/download_artifacts.py YOUR_TOKEN_HERE build_app.yml

# If you want to download artifacts from a specific branch:
python3 scripts/download_artifacts.py YOUR_TOKEN_HERE build_app.yml main
```

## Step 3: Find Your Downloaded Artifacts

The script will:
1. Download the artifacts as zip files to an `artifacts` directory
2. Extract the contents automatically
3. Show you the path where the extracted files are located

You'll find your MoreMojoStudio.app inside:
```
/Volumes/MP 1/MoreMojo/ChatGPT/artifacts/MoreMojoStudio-macOS/MoreMojoStudio.app
```

## Testing Your Token

I've created a simple script to test if your token has the required permissions:

```bash
# Navigate to your project directory
cd "/Volumes/MP 1/MoreMojo/ChatGPT"

# Make the script executable
chmod +x scripts/test_token.sh

# Run the token test
./scripts/test_token.sh YOUR_TOKEN_HERE
```

The script will check if your token can:
- Access repository information
- List workflows
- List workflow runs
- List artifacts

## Troubleshooting

### Authentication Errors

1. **403 Forbidden or Authentication Failed**:
   - Use a classic token (not fine-grained PAT)
   - Ensure token has `repo`, `workflow`, and `read:packages` permissions
   - Double-check that the token hasn't expired

2. **Token Format Issues**:
   - If using a token that starts with `github_pat_`, try creating a classic token instead
   - Don't include any extra spaces or quotes around your token

### Other Common Issues

1. **No workflow runs found**: Make sure you're using the correct workflow name (`build_app.yml` or `build_plugin_mac.yml`)
2. **No artifacts found**: Ensure the workflow completed successfully at least once

## Available Workflows

- `build_app.yml` - Builds the macOS app
- `build_plugin_mac.yml` - Builds the audio plugins
- `test_workflow.yml` - Simple test workflow
