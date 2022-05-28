#include "Memory.h"
#include <iostream>

Memory::Memory(uint32_t init_size) : initial(init_size), maximum(0), memory() {
    memory.reserve(init_size);
}

Memory::Memory(uint32_t init_size, uint32_t max_size) : initial(init_size), maximum(max_size), memory() {
    memory.reserve(init_size);
}

void Memory::setMemory(int index, Variable var) {
    if (index >= memory.size()) {
        if (index <= maximum || maximum == 0) {
            memory.resize(index + 1);
        } else {
            std::cout << "Error: Memory index out of bounds" << std::endl;
            throw;
        }
    }
    memory[index] = var;
}

Variable Memory::getMemory(int index) {
    if (index < memory.size()) {
        return memory[index];
    } else {
        std::cout << "Error: Memory index out of bounds" << std::endl;
        throw;
    }
}

void Memory::fill(uint32_t offset, int32_t value, uint32_t length) {
    if (offset + length >= memory.size()) {
        if (offset + length <= maximum || maximum == 0) {
            memory.resize(offset + length);
        } else {
            std::cout << "Error: Memory index out of bounds" << std::endl;
            throw;
        }
    }
    for (uint32_t i = offset; i < offset + length; i++) {
        memory.emplace(memory.begin() + i, value);
    }
}