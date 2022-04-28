CC=g++

compile:
	$(CC) -g -std=c++2a main.cpp ../includes/*.cpp -o main.out
	wat2wasm --enable-multi-value code.wat
	rm build_test/ -rf
	mkdir build_test
	cp code.wasm build_test/code.wasm
	cd build_test && em++ ../main.cpp ../../includes/*.cpp -sEXPORTED_FUNCTIONS=_useModule -std=c++2a -s WASM=1 -o test.html

execute:
	./main.out

local:
	emrun --no_browser --port 8080 build_test/test.html
