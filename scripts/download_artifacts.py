#!/usr/bin/env python3
"""
GitHub Actions Artifact Downloader

This script downloads the latest successful artifacts from a GitHub Actions workflow.
Usage: python download_artifacts.py <access_token> [workflow_name] [branch_name]

If workflow_name is not provided, it defaults to "Build macOS App"
If branch_name is not provided, it defaults to "main"
"""

import os
import sys
import json
import time
import urllib.request
import zipfile
from urllib.error import HTTPError

def get_workflow_runs(owner, repo, workflow_name, access_token, branch="main"):
    """Get all workflow runs for a specific workflow"""
    url = f"https://api.github.com/repos/{owner}/{repo}/actions/workflows/{workflow_name}/runs?branch={branch}"
    
    # Handle different token formats (classic or fine-grained PAT)
    auth_header = f"Bearer {access_token}" if access_token.startswith("github_pat_") else f"token {access_token}"
    
    headers = {
        "Authorization": auth_header,
        "Accept": "application/vnd.github.v3+json",
        "User-Agent": "MoreMojoArtifactDownloader"
    }
    
    req = urllib.request.Request(url, headers=headers)
    try:
        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read().decode())
            return data.get("workflow_runs", [])
    except HTTPError as e:
        print(f"Error fetching workflow runs: {e}")
        if e.code == 404:
            print("Workflow not found. Available workflows:")
            list_workflows(owner, repo, access_token)
        return []

def list_workflows(owner, repo, access_token):
    """List available workflows in the repository"""
    url = f"https://api.github.com/repos/{owner}/{repo}/actions/workflows"
    
    # Handle different token formats (classic or fine-grained PAT)
    auth_header = f"Bearer {access_token}" if access_token.startswith("github_pat_") else f"token {access_token}"
    
    headers = {
        "Authorization": auth_header,
        "Accept": "application/vnd.github.v3+json",
        "User-Agent": "MoreMojoArtifactDownloader"
    }
    
    req = urllib.request.Request(url, headers=headers)
    try:
        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read().decode())
            workflows = data.get("workflows", [])
            for workflow in workflows:
                print(f"- {workflow['name']} (path: {workflow['path']})")
    except HTTPError as e:
        print(f"Error listing workflows: {e}")

def get_workflow_artifacts(owner, repo, run_id, access_token):
    """Get artifacts for a specific workflow run"""
    url = f"https://api.github.com/repos/{owner}/{repo}/actions/runs/{run_id}/artifacts"
    
    # Handle different token formats (classic or fine-grained PAT)
    auth_header = f"Bearer {access_token}" if access_token.startswith("github_pat_") else f"token {access_token}"
    
    headers = {
        "Authorization": auth_header,
        "Accept": "application/vnd.github.v3+json",
        "User-Agent": "MoreMojoArtifactDownloader"
    }
    
    req = urllib.request.Request(url, headers=headers)
    try:
        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read().decode())
            return data.get("artifacts", [])
    except HTTPError as e:
        print(f"Error fetching artifacts: {e}")
        return []

def download_artifact(owner, repo, artifact_id, artifact_name, access_token):
    """Download a specific artifact by ID"""
    url = f"https://api.github.com/repos/{owner}/{repo}/actions/artifacts/{artifact_id}/zip"
    
    # Handle different token formats (classic or fine-grained PAT)
    auth_header = f"Bearer {access_token}" if access_token.startswith("github_pat_") else f"token {access_token}"
    
    headers = {
        "Authorization": auth_header,
        "Accept": "application/vnd.github.v3+json",
        "User-Agent": "MoreMojoArtifactDownloader"
    }
    
    output_dir = os.path.join(os.getcwd(), "artifacts")
    os.makedirs(output_dir, exist_ok=True)
    output_zip = os.path.join(output_dir, f"{artifact_name}.zip")
    
    print(f"Downloading {artifact_name} to {output_zip}...")
    
    req = urllib.request.Request(url, headers=headers)
    try:
        with urllib.request.urlopen(req) as response:
            with open(output_zip, 'wb') as f:
                f.write(response.read())
        
        print(f"Download complete! Extracting to {output_dir}/{artifact_name}...")
        extract_dir = os.path.join(output_dir, artifact_name)
        os.makedirs(extract_dir, exist_ok=True)
        
        with zipfile.ZipFile(output_zip, 'r') as zip_ref:
            zip_ref.extractall(extract_dir)
        
        print(f"Artifact extracted to {extract_dir}")
        return extract_dir
    except HTTPError as e:
        print(f"Error downloading artifact: {e}")
        return None

def main():
    if len(sys.argv) < 2:
        print("Usage: python download_artifacts.py <access_token> [workflow_name] [branch_name]")
        return 1
    
    access_token = sys.argv[1]
    workflow_name = sys.argv[2] if len(sys.argv) > 2 else "build_app.yml"
    branch = sys.argv[3] if len(sys.argv) > 3 else "main"
    
    owner = "DrGoo1"
    repo = "more-mojo"
    
    print(f"Fetching workflow runs for {workflow_name} on branch {branch}...")
    runs = get_workflow_runs(owner, repo, workflow_name, access_token, branch)
    
    if not runs:
        print(f"No workflow runs found for {workflow_name}")
        return 1
    
    # Find the latest successful run
    successful_runs = [run for run in runs if run['conclusion'] == 'success']
    if not successful_runs:
        print("No successful workflow runs found")
        return 1
    
    latest_run = successful_runs[0]
    run_id = latest_run['id']
    print(f"Latest successful run: #{run_id} ({latest_run['created_at']})")
    
    artifacts = get_workflow_artifacts(owner, repo, run_id, access_token)
    if not artifacts:
        print(f"No artifacts found for run #{run_id}")
        return 1
    
    for artifact in artifacts:
        artifact_id = artifact['id']
        artifact_name = artifact['name']
        print(f"Found artifact: {artifact_name} (id: {artifact_id}, size: {artifact['size_in_bytes']} bytes)")
        download_path = download_artifact(owner, repo, artifact_id, artifact_name, access_token)
        if download_path:
            print(f"You can find the artifact at: {download_path}")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
