.DEFAULT_GOAL := all

CC=g++

compile:
	$(CC) main.cpp bytestream.cpp -o main.out

execute:
	/bin/python3 ./hex_number_generator.py
	./main.out

all: compile execute
