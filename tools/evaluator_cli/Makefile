.PHONY: build run clean

build:
	chmod +x evaluator.swift
	swiftc -O evaluator.swift -o evaluator

run:
	./evaluator --type vocal --plugin "Apple: AUDistortion" --out ./presets/vocal_reco.json

run-all: build
	mkdir -p presets
	./evaluator --type vocal --plugin "Apple: AUDistortion" --out ./presets/vocal_reco.json
	./evaluator --type instrument --plugin "Apple: AUDistortion" --out ./presets/instrument_reco.json
	./evaluator --type drums --plugin "Apple: AUDistortion" --out ./presets/drums_reco.json
	./evaluator --type bus --plugin "Apple: AUDistortion" --out ./presets/bus_reco.json
	./evaluator --type master --plugin "Apple: AUDistortion" --out ./presets/master_reco.json

clean:
	rm -f evaluator
	rm -rf presets/*.json
