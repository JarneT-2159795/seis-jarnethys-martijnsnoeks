.DEFAULT_GOAL := all

CC=g++

compile:
	$(CC) -g -std=c++2a main.cpp ../includes/*.cpp -o main.out

execute:
	wat2wasm test.wat
	./main.out

all: compile execute
