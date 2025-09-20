#!/bin/bash
# Backup script for MoreMojo plugin builds
# This script backs up the current build of the plugin to a timestamped archive

# Create backup directory
BACKUP_DIR="/Volumes/MP 1/MoreMojo/ChatGPT/plugin/backups"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
BACKUP_NAME="MoreMojo_SafeDownsampler_$TIMESTAMP"
BACKUP_PATH="$BACKUP_DIR/$BACKUP_NAME"

# Ensure backup directory exists
mkdir -p "$BACKUP_DIR"

# Create backup directory for this build
mkdir -p "$BACKUP_PATH"

echo "Backing up MoreMojo plugin build to $BACKUP_PATH"

# Copy plugin build artifacts
echo "Copying plugin artifacts..."
cp -R "/Volumes/MP 1/MoreMojo/ChatGPT/plugin/build/MoreMojoPlugin_artefacts" "$BACKUP_PATH/"

# Copy important source files
echo "Backing up source files..."
mkdir -p "$BACKUP_PATH/Source"
cp "/Volumes/MP 1/MoreMojo/ChatGPT/plugin/Source/SafeDownsampler.h" "$BACKUP_PATH/Source/"
cp "/Volumes/MP 1/MoreMojo/ChatGPT/plugin/Source/PluginProcessor.cpp" "$BACKUP_PATH/Source/"
cp "/Volumes/MP 1/MoreMojo/ChatGPT/plugin/Source/PluginProcessor.h" "$BACKUP_PATH/Source/"
cp "/Volumes/MP 1/MoreMojo/ChatGPT/plugin/Source/JuceInit_fix.h" "$BACKUP_PATH/Source/"

# Copy CMake files
cp "/Volumes/MP 1/MoreMojo/ChatGPT/plugin/CMakeLists.txt" "$BACKUP_PATH/"

# Copy documentation
cp "/Volumes/MP 1/MoreMojo/ChatGPT/plugin/SAFE_DOWNSAMPLING_INTEGRATION.md" "$BACKUP_PATH/"

# Create a compressed archive
echo "Creating compressed archive..."
(cd "$BACKUP_DIR" && zip -r "${BACKUP_NAME}.zip" "$BACKUP_NAME")

echo "Backup complete. Files stored at:"
echo "$BACKUP_PATH"
echo "Compressed archive: $BACKUP_DIR/${BACKUP_NAME}.zip"

# For user to find after rebooting
echo "# MoreMojo SafeDownsampler Backup" > "/Volumes/MP 1/MoreMojo/ChatGPT/LAST_BUILD_BACKUP.md"
echo "" >> "/Volumes/MP 1/MoreMojo/ChatGPT/LAST_BUILD_BACKUP.md"
echo "Last build backed up to: $BACKUP_PATH" >> "/Volumes/MP 1/MoreMojo/ChatGPT/LAST_BUILD_BACKUP.md"
echo "Compressed archive: $BACKUP_DIR/${BACKUP_NAME}.zip" >> "/Volumes/MP 1/MoreMojo/ChatGPT/LAST_BUILD_BACKUP.md"
echo "" >> "/Volumes/MP 1/MoreMojo/ChatGPT/LAST_BUILD_BACKUP.md"
echo "Build completed on: $(date)" >> "/Volumes/MP 1/MoreMojo/ChatGPT/LAST_BUILD_BACKUP.md"
echo "" >> "/Volumes/MP 1/MoreMojo/ChatGPT/LAST_BUILD_BACKUP.md"
echo "Plugin locations:" >> "/Volumes/MP 1/MoreMojo/ChatGPT/LAST_BUILD_BACKUP.md"
echo "- AU: /Users/musicpower/Library/Audio/Plug-Ins/Components/More Mojo by Umbo Gumbo.component" >> "/Volumes/MP 1/MoreMojo/ChatGPT/LAST_BUILD_BACKUP.md"
echo "- VST3: /Users/musicpower/Library/Audio/Plug-Ins/VST3/More Mojo by Umbo Gumbo.vst3" >> "/Volumes/MP 1/MoreMojo/ChatGPT/LAST_BUILD_BACKUP.md"
echo "- Standalone: /Volumes/MP 1/MoreMojo/ChatGPT/plugin/build/MoreMojoPlugin_artefacts/Standalone/More Mojo by Umbo Gumbo.app" >> "/Volumes/MP 1/MoreMojo/ChatGPT/LAST_BUILD_BACKUP.md"
