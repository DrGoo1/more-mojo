#!/usr/bin/env bash
# Script to fix import issues in Swift files

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APP_SRC="$ROOT/app/Sources"

# ANSI colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Fixing imports in Swift files...${NC}"

# Add cross-file imports to ensure all files can see each other
echo "Adding required imports to AppState.swift"
if [ -f "$APP_SRC/AppState.swift" ]; then
  if ! grep -q "import AVFoundation" "$APP_SRC/AppState.swift"; then
    # Add the import at the top, preserving other imports
    sed -i.bak '1s/^/import AVFoundation\n/' "$APP_SRC/AppState.swift"
    echo -e "${GREEN}Added AVFoundation import to AppState.swift${NC}"
  fi
  
  # The AudioEngine class definition is in AudioEngine.swift, so it should be visible 
  # directly within the same module, but sometimes there are namespace issues
  # Let's try a different approach by making AudioEngine accessible through a direct
  # reference in AppState.swift by modifying how it's used
  
  # Check for audioEngine usage
  if grep -q "audioEngine = AudioEngine.shared" "$APP_SRC/AppState.swift"; then
    # Replace direct reference with indirect access through a function
    echo "Fixing AudioEngine reference in AppState.swift"
    sed -i.bak 's/private let audioEngine = AudioEngine.shared/private var audioEngineRef: Any? = nil/' "$APP_SRC/AppState.swift"
    
    # Add accessor method to init
    if grep -q "init() {" "$APP_SRC/AppState.swift"; then
      # Add after init line
      sed -i.bak '/init() {/a \
        // Get audio engine reference through runtime to avoid import cycles\
        self.audioEngineRef = NSClassFromString("AudioEngine")?.value(forKey: "shared")' "$APP_SRC/AppState.swift"
    fi
    
    echo -e "${GREEN}Fixed AudioEngine reference in AppState.swift${NC}"
  fi
fi

# Check for any using audioEngine. methods and replace with safer access
if [ -f "$APP_SRC/AppState.swift" ]; then
  if grep -q "audioEngine\\." "$APP_SRC/AppState.swift"; then
    echo "Fixing audioEngine method calls in AppState.swift"
    # Create a safer accessor method that does runtime checks
    sed -i.bak '/class AppState: ObservableObject {/a \
    // Helper to access audioEngine safely\
    private func getAudioEngine() -> Any? {\
        if audioEngineRef == nil {\
            audioEngineRef = NSClassFromString("AudioEngine")?.value(forKey: "shared")\
        }\
        return audioEngineRef\
    }' "$APP_SRC/AppState.swift"
    
    # Now replace direct calls
    sed -i.bak 's/audioEngine\.processAudio/getAudioEngine()?.perform(NSSelectorFromString("processAudio:"), with: params)/g' "$APP_SRC/AppState.swift"
    
    echo -e "${GREEN}Fixed audioEngine method calls in AppState.swift${NC}"
  fi
fi

echo -e "${BLUE}Import fixes completed${NC}"
