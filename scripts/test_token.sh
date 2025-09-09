#!/bin/bash
# Simple script to test GitHub token authentication

if [ -z "$1" ]; then
  echo "Usage: $0 <github_token>"
  exit 1
fi

TOKEN="$1"
OWNER="DrGoo1"
REPO="more-mojo"

# Determine auth header format based on token type
if [[ "$TOKEN" == github_pat_* ]]; then
  AUTH_HEADER="Authorization: Bearer $TOKEN"
else
  AUTH_HEADER="Authorization: token $TOKEN"
fi

echo "Testing token with repository info request..."
curl -s -H "$AUTH_HEADER" -H "Accept: application/vnd.github.v3+json" \
  "https://api.github.com/repos/$OWNER/$REPO" | grep -q "\"name\"" && \
  echo "✓ Token can access repository info" || echo "✗ Token cannot access repository info"

echo "Testing token with workflow list request..."
curl -s -H "$AUTH_HEADER" -H "Accept: application/vnd.github.v3+json" \
  "https://api.github.com/repos/$OWNER/$REPO/actions/workflows" | grep -q "\"workflows\"" && \
  echo "✓ Token can list workflows" || echo "✗ Token cannot list workflows"

echo "Testing token with runs list request..."
curl -s -H "$AUTH_HEADER" -H "Accept: application/vnd.github.v3+json" \
  "https://api.github.com/repos/$OWNER/$REPO/actions/runs" | grep -q "\"workflow_runs\"" && \
  echo "✓ Token can list workflow runs" || echo "✗ Token cannot list workflow runs"

echo "Testing token with artifacts list request..."
curl -s -H "$AUTH_HEADER" -H "Accept: application/vnd.github.v3+json" \
  "https://api.github.com/repos/$OWNER/$REPO/actions/artifacts" | grep -q "\"artifacts\"" && \
  echo "✓ Token can list artifacts" || echo "✗ Token cannot list artifacts"

echo ""
echo "If all tests passed, your token should work with the download script."
echo "If any tests failed, your token might not have the necessary permissions."
