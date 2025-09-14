#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

# Colors for better output
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
BLUE="\033[0;34m"
RED="\033[0;31m"
NC="\033[0m" # No Color

function log_info() { echo -e "${BLUE}[INFO]${NC} $*"; }
function log_warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
function log_error() { echo -e "${RED}[ERROR]${NC} $*"; }
function log_success() { echo -e "${GREEN}[SUCCESS]${NC} $*"; }

# Make sure required tools are available
function check_requirements() {
  local missing_tools=()
  
  if ! command -v gh >/dev/null 2>&1; then
    missing_tools+=("gh")
  fi
  
  if ! command -v python3 >/dev/null 2>&1; then
    missing_tools+=("python3")
  fi
  
  if [[ ${#missing_tools[@]} -gt 0 ]]; then
    log_error "Missing required tools: ${missing_tools[*]}"
    log_info "Please install:"
    [[ " ${missing_tools[*]} " =~ " gh " ]] && echo "  - GitHub CLI: brew install gh"
    [[ " ${missing_tools[*]} " =~ " python3 " ]] && echo "  - Python 3: brew install python"
    exit 1
  fi
}

# Helper to create mock log directories
function create_mock_logs_dir() {
  # Create mock artifacts directory structure
  mkdir -p "$ROOT/failed_artifacts/app-build-logs"
  mkdir -p "$ROOT/failed_artifacts/plugin-build-logs"
  mkdir -p "$ROOT/ci_logs"
}

# Set up test failures for testing specific agents
function setup_test_failure() {
  local failure_type=$1
  log_info "Setting up test failure: $failure_type"
  
  # Always create log directories
  create_mock_logs_dir
  
  case $failure_type in
    swift)
      # Create Swift error by intentionally breaking a file
      if [[ -f "$ROOT/app/Sources/SharedTypes.swift" ]]; then
        cp "$ROOT/app/Sources/SharedTypes.swift" "$ROOT/app/Sources/SharedTypes.swift.bak"
        echo "This will cause a // Swift error" >> "$ROOT/app/Sources/SharedTypes.swift"
        
        # Create mock log with Swift error
        cat > "$ROOT/failed_artifacts/app-build-logs/xcodebuild_app_stdout.log" << EOL
SwiftCompile normal arm64 SharedTypes.swift
    cd /Volumes/MP 1/MoreMojo/ChatGPT/app
    export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
    /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/swiftc -module-name MoreMojoStudio -incremental -emit-dependencies -emit-module -emit-module-path /Volumes/MP 1/MoreMojo/ChatGPT/app/build/Build/Intermediates.noindex/MoreMojoStudio.build/Debug/MoreMojoStudio.build/Objects-normal/arm64/SharedTypes~partial.swiftmodule -output-file-map /Volumes/MP 1/MoreMojo/ChatGPT/app/build/Build/Intermediates.noindex/MoreMojoStudio.build/Debug/MoreMojoStudio.build/SharedTypes-OutputFileMap.json -parse-as-library -c /Volumes/MP 1/MoreMojo/ChatGPT/app/Sources/SharedTypes.swift -serialize-diagnostics -emit-object -o /Volumes/MP 1/MoreMojo/ChatGPT/app/build/Build/Intermediates.noindex/MoreMojoStudio.build/Debug/MoreMojoStudio.build/Objects-normal/arm64/SharedTypes.o
/Volumes/MP 1/MoreMojo/ChatGPT/app/Sources/SharedTypes.swift:119:1: error: cannot find type 'ProcessorParams' in scope
This will cause a // Swift error
^
(5 failures)
EOL
        
        log_success "Added Swift error to SharedTypes.swift and created mock logs"
      else
        log_error "SharedTypes.swift not found"
        return 1
      fi
      ;;
    scheme)
      # Simulate missing scheme issue
      if [[ -d "$ROOT/app/MoreMojoStudio.xcodeproj/xcshareddata/xcschemes" ]]; then
        mv "$ROOT/app/MoreMojoStudio.xcodeproj/xcshareddata/xcschemes" \
           "$ROOT/app/MoreMojoStudio.xcodeproj/xcshareddata/xcschemes.bak"
        
        # Create mock log with scheme error
        cat > "$ROOT/failed_artifacts/app-build-logs/xcodebuild_app_stdout.log" << EOL
xcodebuild: error: The project named "MoreMojoStudio" does not contain a scheme named "MoreMojoStudio". 
The schemes in the project are: (none)
Information about project "MoreMojoStudio":
  Targets:
      MoreMojoStudio
  Configurations:
      Debug
      Release
  Schemes:
      (none)
EOL
        
        log_success "Renamed xcschemes directory and created mock logs"
      else
        log_error "xcschemes directory not found"
        return 1
      fi
      ;;
    cmake)
      # Simulate CMake issue
      if [[ -f "$ROOT/plugin/CMakeLists.txt" ]]; then
        cp "$ROOT/plugin/CMakeLists.txt" "$ROOT/plugin/CMakeLists.txt.bak"
        echo "\$<TARGET_BUNDLE_DIR:MoreMojoPlugin>" >> "$ROOT/plugin/CMakeLists.txt"
        
        # Create mock log with CMake error
        cat > "$ROOT/failed_artifacts/plugin-build-logs/cmake_configure.log" << EOL
-- Configuring MoreMojoPlugin...
-- Found JUCE: /Users/runner/work/more-mojo/more-mojo/JUCE/extras/Build/CMake/JUCEConfig.cmake
CMake Error at CMakeLists.txt:41 (add_library):
  Error evaluating generator expression:

    \$<TARGET_BUNDLE_DIR:MoreMojoPlugin>

  \$<TARGET_BUNDLE_DIR:...> may only be used with binary targets.
-- Configuring incomplete, errors occurred!
See also "/Volumes/MP 1/MoreMojo/ChatGPT/plugin/build/CMakeFiles/CMakeOutput.log".
EOL
        
        log_success "Added problematic CMake expression and created mock logs"
      else
        log_error "CMakeLists.txt not found"
        return 1
      fi
      ;;
    *)
      log_error "Unknown failure type: $failure_type"
      echo "Available types: swift, scheme, cmake"
      return 1
      ;;
  esac
}

# Revert test failures after testing
function cleanup_test_failure() {
  local failure_type=$1
  log_info "Cleaning up test failure: $failure_type"
  
  case $failure_type in
    swift)
      if [[ -f "$ROOT/app/Sources/SharedTypes.swift.bak" ]]; then
        mv "$ROOT/app/Sources/SharedTypes.swift.bak" "$ROOT/app/Sources/SharedTypes.swift"
        log_success "Restored SharedTypes.swift"
      fi
      ;;
    scheme)
      if [[ -d "$ROOT/app/MoreMojoStudio.xcodeproj/xcshareddata/xcschemes.bak" ]]; then
        mv "$ROOT/app/MoreMojoStudio.xcodeproj/xcshareddata/xcschemes.bak" \
           "$ROOT/app/MoreMojoStudio.xcodeproj/xcshareddata/xcschemes"
        log_success "Restored xcschemes directory"
      fi
      ;;
    cmake)
      if [[ -f "$ROOT/plugin/CMakeLists.txt.bak" ]]; then
        mv "$ROOT/plugin/CMakeLists.txt.bak" "$ROOT/plugin/CMakeLists.txt"
        log_success "Restored CMakeLists.txt"
      fi
      ;;
    *)
      log_error "Unknown failure type: $failure_type"
      echo "Available types: swift, scheme, cmake"
      return 1
      ;;
  esac
}

# Parse command line arguments
function show_usage() {
  echo "Usage: $(basename "$0") [options]"
  echo ""
  echo "Options:"
  echo "  --download                Download latest CI artifacts (requires GitHub CLI)"
  echo "  --workflow <workflow>     GitHub workflow name to download artifacts from"
  echo "  --outdir <path>          Directory to store downloaded artifacts"
  echo "  --test-failure <type>     Create test failure (swift, scheme, cmake)"
  echo "  --cleanup <type>          Clean up test failure (swift, scheme, cmake)"
  echo "  --local-logs              Use local logs instead of downloading from CI"
  echo "  --help                   Show this help message"
  echo ""
  echo "Example:"
  echo "  $(basename "$0") --download --workflow 'Build App & Plugins (macOS) with Logs'"
  echo "  $(basename "$0") --test-failure swift"
  echo "  $(basename "$0") --cleanup swift"
  echo "  $(basename "$0") --local-logs"
}

# Default values
WORKFLOW="Build App & Plugins (macOS) with Logs"
OUTDIR="$ROOT/failed_artifacts_local"
DOWNLOAD=false
TEST_FAILURE=""
CLEANUP=""
LOCAL_LOGS=false

# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    --download)
      DOWNLOAD=true
      shift
      ;;
    --workflow)
      WORKFLOW="$2"
      shift 2
      ;;
    --outdir)
      OUTDIR="$2"
      shift 2
      ;;
    --test-failure)
      TEST_FAILURE="$2"
      shift 2
      ;;
    --cleanup)
      CLEANUP="$2"
      shift 2
      ;;
    --local-logs)
      LOCAL_LOGS=true
      shift
      ;;
    --help)
      show_usage
      exit 0
      ;;
    *)
      log_error "Unknown option: $1"
      show_usage
      exit 1
      ;;
  esac
done

# Check requirements
check_requirements

# Process test failures
if [[ -n "$TEST_FAILURE" ]]; then
  setup_test_failure "$TEST_FAILURE"
  exit $?
fi

# Process cleanup
if [[ -n "$CLEANUP" ]]; then
  cleanup_test_failure "$CLEANUP"
  exit $?
fi

# Prepare output directory
mkdir -p "$OUTDIR"

# Download artifacts if requested
if [[ "$DOWNLOAD" = true ]]; then
  log_info "Downloading artifacts from workflow: '$WORKFLOW'"
  RUN_ID=$(gh run list --workflow "$WORKFLOW" -L 1 --json databaseId -q '.[0].databaseId')
  if [[ -z "$RUN_ID" ]]; then
    log_error "No workflow runs found for: $WORKFLOW"
    exit 1
  fi
  log_info "Found run ID: $RUN_ID"
  gh run download "$RUN_ID" -D "$OUTDIR"
  log_success "Downloaded artifacts to: $OUTDIR"
fi

# Set up local logs if using local mode
if [[ "$LOCAL_LOGS" = true ]]; then
  log_info "Using local logs mode"
  # Create symlinks from failed_artifacts to the local directory
  rm -rf "$ROOT/failed_artifacts"
  ln -sf "$OUTDIR" "$ROOT/failed_artifacts"
  log_success "Linked local logs directory"
fi

# Run agent_hub.py
log_info "Running agent hub..."
python3 "$ROOT/scripts/swarm/agent_hub.py"

# Show changes
log_info "Changes detected by Swarm:"
echo "== Diff =="
git -C "$ROOT" status --porcelain=v1 || true
git -C "$ROOT" diff --stat || true

if git -C "$ROOT" diff --quiet; then
  log_info "No changes made by Swarm agents"
else
  log_success "Swarm agents made changes! Review the diff above."
  echo ""
  echo "To commit these changes:"
  echo "  git add -A"
  echo "  git commit -m 'Swarm: <description of fixes>'"
  echo "  git push -u origin <branch>"
fi
