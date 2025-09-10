#!/usr/bin/env bash
set -eo pipefail

# CI Build Error Reporter
# Usage: ./ci_error_report.sh [error_log_file]

echo "===== CI Build Error Report ====="

# Get directory containing this script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
ERROR_LOG="${1:-$PROJECT_ROOT/build_error.log}"
REPORT_FILE="$PROJECT_ROOT/ci_error_report.txt"

# Create report header
cat > "$REPORT_FILE" << EOH
====================================================
MoreMojo CI Build Error Report
Generated: $(date)
====================================================

EOH

# Check system environment
echo "Checking system environment..." | tee -a "$REPORT_FILE"
{
  echo "macOS Version: $(sw_vers -productVersion 2>/dev/null || echo "Unknown")"
  echo "Xcode Version: $(xcodebuild -version 2>/dev/null | head -n1 || echo "Unknown")"
  echo "Swift Version: $(swift --version 2>/dev/null | head -n1 || echo "Unknown")"
} >> "$REPORT_FILE"

# Check project structure
echo "Checking project structure..." | tee -a "$REPORT_FILE"
{
  echo "Sources files: $(find "$PROJECT_ROOT/app/Sources" -name "*.swift" 2>/dev/null | wc -l | tr -d ' ')"
  echo "Resource files: $(find "$PROJECT_ROOT/app/Resources" -type f 2>/dev/null | wc -l | tr -d ' ')"
  
  # Check for critical files
  critical_files=(
    "app/Sources/MoreMojoStudioApp.swift"
    "app/Sources/ContentView.swift"
    "app/Sources/AudioEngine.swift"
    "app/Sources/ProcessorParams.swift"
  )
  
  echo -e "\nCritical files check:"
  for file in "${critical_files[@]}"; do
    if [ -f "$PROJECT_ROOT/$file" ]; then
      echo "✓ $file: Present"
    else
      echo "✗ $file: MISSING"
    fi
  done
} >> "$REPORT_FILE"

# Parse error log if it exists
if [ -f "$ERROR_LOG" ]; then
  echo "Analyzing build errors..." | tee -a "$REPORT_FILE"
  {
    echo -e "\nError Summary:"
    
    # Extract Swift compiler errors
    swift_errors=$(grep -A 2 "error:" "$ERROR_LOG" | grep -v "note:" || echo "No Swift errors found")
    echo -e "\nSwift Compiler Errors:"
    echo "$swift_errors"
    
    # Extract linker errors
    linker_errors=$(grep -A 2 "ld:" "$ERROR_LOG" | grep -v "warning:" || echo "No linker errors found")
    echo -e "\nLinker Errors:"
    echo "$linker_errors"
    
    # Extract resource errors
    resource_errors=$(grep -A 2 "resource" "$ERROR_LOG" | grep "error" || echo "No resource errors found")
    echo -e "\nResource Errors:"
    echo "$resource_errors"
    
    # Count error types
    echo -e "\nError Statistics:"
    echo "Swift errors: $(echo "$swift_errors" | grep -c "error:" || echo 0)"
    echo "Linker errors: $(echo "$linker_errors" | grep -c "ld:" || echo 0)"
    echo "Resource errors: $(echo "$resource_errors" | grep -c "error" || echo 0)"
  } >> "$REPORT_FILE"
else
  echo "No error log found at $ERROR_LOG" | tee -a "$REPORT_FILE"
fi

# Recommendations section
echo "Generating recommendations..." | tee -a "$REPORT_FILE"
{
  echo -e "\n====================================================\nRecommendations:"
  
  # Check for common issues and recommend solutions
  if grep -q "Cannot find 'FaceplateLayout'" "$ERROR_LOG" 2>/dev/null; then
    echo "- Run scripts/asset_check.sh to create missing FaceplateLayout files"
  fi
  
  if grep -q "No such module" "$ERROR_LOG" 2>/dev/null; then
    echo "- Make sure all required dependencies are included in project.yml"
  fi
  
  if grep -q "Undefined symbol" "$ERROR_LOG" 2>/dev/null; then
    echo "- Check for missing implementation files or linking issues"
  fi
  
  if grep -q "Could not find bundled resource" "$ERROR_LOG" 2>/dev/null; then
    echo "- Add missing resource files to app/Resources directory"
  fi
  
  echo "- Use scripts/ci_build_wrapper.sh to fall back to a minimal viable app if build fails"
  echo "- Consider using .github/workflows/ci_only_build.yml for CI builds"
} >> "$REPORT_FILE"

echo "Error report created at: $REPORT_FILE"
echo "===== Error report complete ====="
