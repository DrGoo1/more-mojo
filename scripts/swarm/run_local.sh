#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

# Fetch latest CI artifacts locally (requires gh CLI)
WORKFLOW="${1:-Build App & Plugins (macOS) with Logs}"
OUTDIR="${2:-$ROOT/failed_artifacts_local}"
mkdir -p "$OUTDIR"
RUN_ID=$(gh run list --workflow "$WORKFLOW" -L 1 --json databaseId -q '.[0].databaseId')
gh run download "$RUN_ID" -D "$OUTDIR"

python3 "$ROOT/scripts/swarm/agent_hub.py"

echo "== Diff =="
git -C "$ROOT" status --porcelain=v1 || true
git -C "$ROOT" diff --stat || true
echo "If changes look good: git add -A && git commit -m 'Swarm: fixes' && git push -u origin <branch>"
