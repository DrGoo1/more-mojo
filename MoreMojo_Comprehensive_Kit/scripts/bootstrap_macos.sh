
#!/usr/bin/env bash
set -euo pipefail

# Xcode Command Line Tools
if ! xcode-select -p >/dev/null 2>&1; then
  xcode-select --install || true
  echo "If prompted, complete Xcode CLT installation then rerun bootstrap."
fi

# Homebrew + deps
if ! command -v brew >/dev/null 2>&1; then
  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

brew list xcodegen >/dev/null 2>&1 || brew install xcodegen
brew list cmake    >/dev/null 2>&1 || brew install cmake

echo "bootstrap complete"
