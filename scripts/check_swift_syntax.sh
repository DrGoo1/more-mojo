#!/usr/bin/env bash
# Script to check Swift syntax for common errors

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APP_SRC="$ROOT/app/Sources"

# ANSI colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Checking Swift syntax in $APP_SRC${NC}"

# First check if swiftlint is available
if command -v swiftlint &> /dev/null; then
    echo -e "${GREEN}SwiftLint found, using for syntax checking${NC}"
    # Run swiftlint in autocorrect mode if available
    swiftlint autocorrect --path "$APP_SRC" || echo -e "${YELLOW}SwiftLint autocorrect had issues${NC}"
    swiftlint --path "$APP_SRC" || echo -e "${YELLOW}SwiftLint found issues${NC}"
else
    echo -e "${YELLOW}SwiftLint not found, using manual checks${NC}"
fi

# Manual checks for common Swift errors
echo -e "${BLUE}Running manual syntax checks:${NC}"

# Check for imports
echo "1. Checking for missing imports"
for file in "$APP_SRC"/*.swift; do
    filename=$(basename "$file")
    if ! grep -q "^import " "$file"; then
        echo -e "${RED}  - $filename: Missing import statements${NC}"
    fi
done

# Check for common Swift syntax errors
echo "2. Checking for common syntax issues"

# Check for unbalanced braces
for file in "$APP_SRC"/*.swift; do
    filename=$(basename "$file")
    open_braces=$(grep -o "{" "$file" | wc -l)
    close_braces=$(grep -o "}" "$file" | wc -l)
    
    if [ "$open_braces" != "$close_braces" ]; then
        echo -e "${RED}  - $filename: Unbalanced braces! { = $open_braces, } = $close_braces${NC}"
    fi
done

# Check for Swift reserved words used as identifiers
echo "3. Checking for reserved words used as identifiers"
reserved_words=("class" "struct" "enum" "protocol" "var" "let" "func" "init" "self" "super" "true" "false" "nil")

for file in "$APP_SRC"/*.swift; do
    filename=$(basename "$file")
    for word in "${reserved_words[@]}"; do
        # Look for the word used as an identifier (preceded by a space, followed by a colon or space)
        if grep -q "[^a-zA-Z0-9_]$word[ :]" "$file"; then
            echo -e "${YELLOW}  - $filename: Possible misuse of reserved word '$word'${NC}"
        fi
    done
done

# Check for missing protocol conformances
echo "4. Checking for protocol implementation issues"
for file in "$APP_SRC"/*.swift; do
    filename=$(basename "$file")
    
    # Check for protocols
    if grep -q ": *.*Protocol" "$file"; then
        protocols=$(grep -o ": *[A-Za-z0-9_, ]*Protocol" "$file" | sed 's/: *//' | tr ',' '\n' | tr ' ' '\n' | grep "Protocol")
        
        for protocol in $protocols; do
            # Simplified check - in real life this would be more sophisticated
            if ! grep -q "func [a-zA-Z0-9_]*" "$file"; then
                echo -e "${YELLOW}  - $filename: References protocol $protocol but may be missing method implementations${NC}"
            fi
        done
    fi
done

# Check for missing super.init() calls in initializers
echo "5. Checking for missing super.init() calls"
for file in "$APP_SRC"/*.swift; do
    filename=$(basename "$file")
    
    # If file has a class or struct that inherits
    if grep -q "class .* *: " "$file"; then
        # If it has an init but no super.init
        if grep -q "init" "$file" && ! grep -q "super\.init" "$file"; then
            echo -e "${YELLOW}  - $filename: Has initializer but may be missing super.init() call${NC}"
        fi
    fi
done

# Check for files using vDSP without importing Accelerate
echo "6. Checking for Accelerate framework usage"
for file in "$APP_SRC"/*.swift; do
    filename=$(basename "$file")
    
    # If file uses vDSP but doesn't import Accelerate
    if grep -q "vDSP" "$file" && ! grep -q "import Accelerate" "$file"; then
        echo -e "${RED}  - $filename: Uses vDSP but is missing 'import Accelerate'${NC}"
    fi
done

# Swift-specific syntax check using swift command if available
if command -v swift &> /dev/null; then
    echo "7. Using swift command for syntax checking"
    
    # Create a temporary directory for syntax checking
    tmp_dir=$(mktemp -d)
    trap 'rm -rf "$tmp_dir"' EXIT
    
    # Check each file independently
    for file in "$APP_SRC"/*.swift; do
        filename=$(basename "$file")
        tmp_file="$tmp_dir/$filename"
        
        # Add minimum imports to make syntax checking work
        cat > "$tmp_file" << EOF
import Foundation
import SwiftUI
import Accelerate
import Combine
import AVFoundation

// Original file content follows:
$(cat "$file")
EOF
        
        # Try to parse the file with swift
        if ! swift "$tmp_file" -parse &>/dev/null; then
            echo -e "${RED}  - $filename: Swift parser found syntax errors${NC}"
            
            # More detailed error output
            swift "$tmp_file" -parse 2>&1 | grep -v "warning:" | head -n 5
        fi
    done
    
    echo "Syntax check completed"
else
    echo -e "${YELLOW}Swift command not available for syntax checking${NC}"
fi

echo -e "${BLUE}Swift syntax check completed${NC}"
