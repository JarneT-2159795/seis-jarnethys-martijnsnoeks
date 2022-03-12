.DEFAULT_GOAL := all

CC=g++

compile:
	$(CC) -g main.cpp ../includes/*.cpp -o main.out

execute:
	wat2wasm test.wat
	./main.out

all: compile execute
