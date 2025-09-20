#!/bin/bash
# Script to create a release bundle for "More Like The Record" plugin
# Includes all source, CMake files, resources, and documentation in one zip file

# Exit on error
set -e

# Print colored status messages
print_status() {
    echo -e "\033[1;34m[BUNDLE]\033[0m $1"
}

print_success() {
    echo -e "\033[1;32m[SUCCESS]\033[0m $1"
}

print_error() {
    echo -e "\033[1;31m[ERROR]\033[0m $1"
    exit 1
}

# Check version (default to v3.0.0 if not specified)
VERSION="${1:-v3.0.0}"
RELEASE_NAME="MoreLikeTheRecord_${VERSION}"
BUNDLE_NAME="${RELEASE_NAME}.zip"

print_status "Creating release bundle: $BUNDLE_NAME"

# Create a temporary directory for the bundle contents
TEMP_DIR=$(mktemp -d)
BUNDLE_DIR="$TEMP_DIR/$RELEASE_NAME"

print_status "Preparing bundle contents in $BUNDLE_DIR..."
mkdir -p "$BUNDLE_DIR"

# Copy source files
print_status "Copying source files..."
cp -r Source "$BUNDLE_DIR/"

# Copy resources
print_status "Copying resource files..."
mkdir -p "$BUNDLE_DIR/Resources"
if [ -d "Resources" ]; then
    cp -r Resources "$BUNDLE_DIR/"
else
    mkdir -p "$BUNDLE_DIR/Resources/icons"
    print_status "Resources directory not found. Created an empty one."
fi

# Copy build files
print_status "Copying build files..."
cp CMakeLists.txt "$BUNDLE_DIR/"
cp build_plugin.sh "$BUNDLE_DIR/"
chmod +x "$BUNDLE_DIR/build_plugin.sh"

# Copy documentation
print_status "Copying documentation..."
cp README.md "$BUNDLE_DIR/"

# Create additional helper files
print_status "Creating additional helper files..."

# Create a basic .gitignore
cat > "$BUNDLE_DIR/.gitignore" << EOL
# Build directories
/build/
/cmake-build-*/

# IDE files
/.idea/
/.vscode/
/.vs/
*.user
*.xcworkspace
*.xcuserstate

# macOS files
.DS_Store
.AppleDouble
.LSOverride

# Temporary files
*.swp
*~
EOL

# Create placeholder for icons
if [ ! -d "$BUNDLE_DIR/Resources/icons" ] || [ -z "$(ls -A "$BUNDLE_DIR/Resources/icons")" ]; then
    mkdir -p "$BUNDLE_DIR/Resources/icons"
    
    # Create a README in the icons directory
    cat > "$BUNDLE_DIR/Resources/icons/README.txt" << EOL
Place your SVG icons here.
These will be automatically embedded into the plugin binary during build.

Recommended icons:
- mojo_logo.svg
- slider_knob.svg
- button_on.svg
- button_off.svg
EOL
fi

# Create a version file
echo "$VERSION" > "$BUNDLE_DIR/VERSION"

# Create the zip bundle
print_status "Creating zip bundle..."
(cd "$TEMP_DIR" && zip -r "$BUNDLE_NAME" "$RELEASE_NAME")

# Move the bundle to the current directory
mv "$TEMP_DIR/$BUNDLE_NAME" ./

# Clean up the temporary directory
rm -rf "$TEMP_DIR"

print_success "Release bundle created: $(pwd)/$BUNDLE_NAME"
print_success "Bundle contains everything needed for building the plugin."
print_success "To build, extract the bundle, navigate to the directory, and run: ./build_plugin.sh"
