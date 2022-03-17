.DEFAULT_GOAL := all

CC=g++

compile:
	$(CC) -g main.cpp ../includes/bytestream.cpp -o main.out

execute:
	/bin/python3 ./hex_number_generator.py
	./main.out

all: compile execute
