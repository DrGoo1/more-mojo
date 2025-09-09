
PROJECT := RealFeel\ Studio.xcodeproj
DERIVED := build

.PHONY: gen build run clean

gen:
	@which xcodegen >/dev/null 2>&1 || brew install xcodegen
	xcodegen generate

build: gen
	xcodebuild -project "$(PROJECT)" -scheme "RealFeel Studio" -configuration Release -derivedDataPath "$(DERIVED)" clean build

run:
	open "$(DERIVED)/Build/Products/Release/RealFeel Studio.app"

clean:
	rm -rf "$(DERIVED)"
