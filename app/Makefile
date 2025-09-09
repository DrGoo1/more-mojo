# MoreMojo App Makefile

.PHONY: all build run clean

all: build

# Generate Xcode project using XcodeGen
project: project.yml
	@echo "Generating Xcode project..."
	@xcodegen generate || ../bin/xcodegen generate

# Build the app
build: project
	@echo "Building MoreMojo Studio app..."
	@xcodebuild -project MoreMojoStudio.xcodeproj -scheme MoreMojoStudio -configuration Release -derivedDataPath build

# Run the app
run: build
	@echo "Running MoreMojo Studio app..."
	@open build/Build/Products/Release/MoreMojoStudio.app

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf build
	@rm -rf MoreMojoStudio.xcodeproj
	@echo "Cleanup complete!"
