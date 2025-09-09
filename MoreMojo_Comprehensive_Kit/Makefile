
.PHONY: bootstrap app app-run plugin plugin-install evaluate package clean

bootstrap:
	./scripts/bootstrap_macos.sh

app:
	$(MAKE) -C app build

app-run:
	$(MAKE) -C app run

plugin:
	cd plugin && cmake -B build -S . -DJUCE_DIR=$$JUCE_DIR && cmake --build build --config Release

plugin-install:
	./plugin/scripts/install_plugin.sh plugin/build/Release

evaluate:
	@echo "Evaluator (Phase 3) will run here."

package:
	mkdir -p dist
	-cp -R "app/build/Build/Products/Release/RealFeel Studio.app" dist/
	-cp -R plugin/build/Release/*.component dist/ 2>/dev/null || true
	-cp -R plugin/build/Release/*.vst3 dist/ 2>/dev/null || true
	@echo "Artifacts in ./dist"

clean:
	$(MAKE) -C app clean || true
	rm -rf plugin/build dist
