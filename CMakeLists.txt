cmake_minimum_required(VERSION 3.22)
project(seis_jarnethys_martijnsnoeks)

set(CMAKE_CXX_STANDARD 20)

include_directories(includes)
include_directories(test-module)

add_executable(seis_jarnethys_martijnsnoeks
        includes/bytestream.cpp
        includes/bytestream.h
        includes/compiler.cpp
        includes/compiler.h
        includes/constants.h
        includes/function.cpp
        includes/function.h
        includes/instruction.h
        includes/lexer.cpp
        includes/lexer.h
        includes/module.cpp
        includes/module.h
        includes/parser.cpp
        includes/parser.h
        includes/stack.cpp
        includes/stack.h
        includes/token.h
        test-compiler/main.cpp
)
