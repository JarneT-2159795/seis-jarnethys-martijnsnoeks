local:
	wat2wasm --enable-multi-value code.wat
	rm build_test/ -rf
	mkdir build_test
	cp code.wasm build_test/code.wasm
	cd build_test && em++ ../main.cpp ../../includes/*.cpp -sEXPORTED_FUNCTIONS=_useModule -std=c++2a -s WASM=1 -o test.html
	emrun --no_browser --port 8080 build_test/test.html

edge-local:
	wat2wasm --enable-multi-value code.wat
	rm build_edge/ -rf
	mkdir build_edge
	cd build_edge && em++ ../main.cpp ../../includes/*.cpp -std=c++2a --no-entry -O2 -s WASM=1 -s EXPORTED_FUNCTIONS="[_useModule, _randomInt, _compile]" -s ALLOW_MEMORY_GROWTH=1 -s DYNAMIC_EXECUTION=0 -s TEXTDECODER=0 -s MODULARIZE=1 -s ENVIRONMENT='web' -s EXPORT_NAME="WASMModule" --pre-js '../pre.js' -o test.js
	cp build_edge/test.js vm-worker/build/test.js
	cp build_edge/test.wasm vm-worker/build/test.wasm
	cd vm-worker && wrangler dev

edge-publish:
	wat2wasm --enable-multi-value code.wat
	rm build_edge/ -rf
	mkdir build_edge
	cd build_edge && em++ ../main.cpp ../../includes/*.cpp -std=c++2a --no-entry -O2 -s WASM=1 -s EXPORTED_FUNCTIONS="[_useModule, _randomInt, _compile]" -s ALLOW_MEMORY_GROWTH=1 -s DYNAMIC_EXECUTION=0 -s TEXTDECODER=0 -s MODULARIZE=1 -s ENVIRONMENT='web' -s EXPORT_NAME="WASMModule" --pre-js '../pre.js' -o test.js
	cp build_edge/test.js vm-worker/build/test.js
	cp build_edge/test.wasm vm-worker/build/test.wasm
	cd vm-worker && wrangler publish
