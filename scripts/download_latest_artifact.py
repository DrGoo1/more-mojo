#!/usr/bin/env python3
"""
Direct GitHub artifact downloader using token authentication
For use when GitHub CLI (gh) is not available
"""

import os
import sys
import json
import requests
import time
from pathlib import Path
import zipfile

# Configuration
OWNER = "DrGoo1"
REPO = "more-mojo"
WORKFLOW_NAME = "Build macOS App"
ARTIFACT_NAME = "MoreMojo-App"
OUTPUT_DIR = Path("./downloads")


def get_token():
    """Get GitHub token from user input or environment"""
    token = os.environ.get("GITHUB_TOKEN")
    if not token:
        token = input("Enter GitHub token: ")
    return token


def get_latest_run_id(token, workflow_name):
    """Get the latest successful run ID for a workflow"""
    url = f"https://api.github.com/repos/{OWNER}/{REPO}/actions/workflows/{workflow_name}/runs"
    headers = {
        "Authorization": f"token {token}",
        "Accept": "application/vnd.github.v3+json"
    }
    
    response = requests.get(url, headers=headers)
    if response.status_code != 200:
        print(f"Error fetching workflow runs: {response.status_code}")
        print(response.text)
        return None
    
    data = response.json()
    for run in data.get("workflow_runs", []):
        if run["status"] == "completed" and run["conclusion"] == "success":
            return run["id"]
    
    print("No successful workflow runs found")
    return None


def list_artifacts(token, run_id):
    """List artifacts for a specific run"""
    url = f"https://api.github.com/repos/{OWNER}/{REPO}/actions/runs/{run_id}/artifacts"
    headers = {
        "Authorization": f"token {token}",
        "Accept": "application/vnd.github.v3+json"
    }
    
    response = requests.get(url, headers=headers)
    if response.status_code != 200:
        print(f"Error fetching artifacts: {response.status_code}")
        print(response.text)
        return []
    
    return response.json().get("artifacts", [])


def download_artifact(token, artifact_id, artifact_name, output_dir):
    """Download a specific artifact by ID"""
    url = f"https://api.github.com/repos/{OWNER}/{REPO}/actions/artifacts/{artifact_id}/zip"
    headers = {"Authorization": f"token {token}"}
    
    output_dir.mkdir(parents=True, exist_ok=True)
    output_zip = output_dir / f"{artifact_name}.zip"
    
    print(f"Downloading {artifact_name} to {output_zip}...")
    
    response = requests.get(url, headers=headers, stream=True)
    if response.status_code != 200:
        print(f"Error downloading artifact: {response.status_code}")
        print(response.text)
        return None
    
    with open(output_zip, 'wb') as f:
        for chunk in response.iter_content(chunk_size=8192):
            if chunk:
                f.write(chunk)
    
    print(f"Download complete! Extracting to {output_dir}/{artifact_name}/...")
    extract_dir = output_dir / artifact_name
    extract_dir.mkdir(parents=True, exist_ok=True)
    
    try:
        with zipfile.ZipFile(output_zip, 'r') as zip_ref:
            zip_ref.extractall(extract_dir)
        print(f"Artifact extracted to {extract_dir}")
        return extract_dir
    except zipfile.BadZipFile:
        print("Warning: Downloaded file is not a valid zip file. It may be empty or corrupted.")
        print(f"Raw file saved at {output_zip}")
        return output_zip


def main():
    token = get_token()
    if not token:
        print("Error: GitHub token is required")
        return 1
    
    print(f"Finding latest successful run for {WORKFLOW_NAME}...")
    run_id = get_latest_run_id(token, WORKFLOW_NAME)
    if not run_id:
        return 1
    
    print(f"Found run ID: {run_id}")
    print(f"Listing artifacts for run {run_id}...")
    artifacts = list_artifacts(token, run_id)
    
    if not artifacts:
        print(f"No artifacts found for run {run_id}")
        return 1
    
    target_artifact = None
    for artifact in artifacts:
        print(f"Found artifact: {artifact['name']} (ID: {artifact['id']})")
        if artifact['name'] == ARTIFACT_NAME:
            target_artifact = artifact
    
    if not target_artifact:
        print(f"Artifact '{ARTIFACT_NAME}' not found")
        return 1
    
    artifact_path = download_artifact(token, target_artifact['id'], target_artifact['name'], OUTPUT_DIR)
    if artifact_path:
        print("\nSuccess!")
        print(f"Artifact downloaded and extracted to: {artifact_path}")
        return 0
    
    return 1


if __name__ == "__main__":
    sys.exit(main())
