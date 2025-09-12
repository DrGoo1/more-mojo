#!/usr/bin/env bash
set -euo pipefail
WORKFLOW="${1:-Build App & Plugins (macOS) with Logs}"
OUTDIR="${2:-./ci_latest}"
mkdir -p "$OUTDIR"
RUN_ID=$(gh run list --workflow "$WORKFLOW" -L 1 --json databaseId,conclusion -q '.[0].databaseId')
if [ -z "${RUN_ID:-}" ]; then echo "No runs for $WORKFLOW"; exit 1; fi
gh run download "$RUN_ID" -D "$OUTDIR"
echo "Artifacts downloaded to $OUTDIR"
