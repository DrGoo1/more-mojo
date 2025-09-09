.PHONY: bootstrap app app-run plugin plugin-install package clean

bootstrap:
	./scripts/bootstrap_macos.sh

app:
	./scripts/build_app.sh

app-run:
	$(MAKE) -C app run

plugin:
	./scripts/build_plugin.sh

plugin-install:
	./plugin/scripts/install_plugin.sh plugin/build/Release

package:
	mkdir -p dist
	-cp -R "app/build/Build/Products/Release/MoreMojoStudio.app" dist/
	-cp -R plugin/build/Release/*.component dist/ 2>/dev/null || true
	-cp -R plugin/build/Release/*.vst3 dist/ 2>/dev/null || true
	@echo "Artifacts in ./dist"

clean:
	$(MAKE) -C app clean || true
	rm -rf plugin/build dist
