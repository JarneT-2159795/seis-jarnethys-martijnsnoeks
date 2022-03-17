#include <fstream>
#include "bytestream.h"
#include <iostream>
#include <iomanip>
#include <bitset>
#include <stdexcept>

ByteStream::ByteStream(std::string filepath) : currentByteIndex{ 0 } {
    std::ifstream f(filepath, std::ios::binary);
    if (f.is_open()) {
        // Get length of file:
        f.seekg (0, f.end);
        size = f.tellg();
        f.seekg (0, f.beg);

        buffer = new uint8_t[size];
        f.read((char*)buffer, size);
    }
    f.close();
}

ByteStream::ByteStream(std::vector<uint8_t> stream) : currentByteIndex{ 0 } {
    size = stream.size();
    buffer = new uint8_t[size];
    for (int i = 0; i < size; ++i) {
        buffer[i] = stream[i];
    }
}

ByteStream::~ByteStream() {
    if (buffer != nullptr) {
        delete buffer;
    }
}

void ByteStream::readFile(std::string filepath) {
    if (buffer != nullptr) {
        delete buffer;
    }
    currentByteIndex = 0;
    std::ifstream f(filepath, std::ios::binary);
    if (f.is_open()) {
        // Get length of file:
        f.seekg (0, f.end);
        size = f.tellg();
        f.seekg (0, f.beg);

        buffer = new uint8_t[size];
        f.read((char*)buffer, size);
    }
    f.close();
}

uint8_t ByteStream::readByte() {
    if (atEnd()) {
        throw std::out_of_range("ByteStream at end of stream");
    }
    ++currentByteIndex;
    return buffer[currentByteIndex - 1];
}

std::string ByteStream::readASCIIString(int length) {
    std::string s{""};

    for (int i = 0; i < length; ++i) {
        s += readByte();
    }

    return s;
}

int32_t ByteStream::readInt32() {
    int32_t result{0};
    int shift{0};
    uint8_t byte;

    do
    {
        byte = readByte();
        result |= ((byte & 0b0111'1111) << shift);
        shift += 7;
    } while ((byte & 0b1000'0000) != 0);

    if ((shift < 32) && ((byte & 0b0100'0000) == 0b0100'0000)) {
        result |= (~0 << shift);
    }

    return result;
}

int64_t ByteStream::readInt64() {
    int64_t result{0};
    int shift{0};
    uint8_t byte;

    do
    {
        byte = readByte();
        result |= ((int64_t)(byte & 0b0111'1111) << shift);
        shift += 7;
    } while ((byte & 0b1000'0000) != 0);

    if ((shift < 64) && ((byte & 0b0100'0000) == 0b0100'0000)) {
        result |= ((int64_t)~0 << shift);
    }

    return result;
}

uint32_t ByteStream::readUInt32() {
    uint32_t result{0};
    int shift{0};

    while (true) {
        uint8_t byte = readByte();
        result |= ((byte & 0b0111'1111) << shift);
        if ((byte & 0b1000'0000) == 0) {
            break;
        }
        shift += 7;
    }

    return result;
}

uint64_t ByteStream::readUInt64() {
    uint64_t result{0};
    int shift{0};

    while (true) {
        uint8_t byte = readByte();
        result |= ((uint64_t)(byte & 0b0111'1111) << shift);
        if ((byte & 0b1000'0000) == 0) {
            break;
        }
        shift += 7;
    }
    return result;
}

float32_t ByteStream::readFloat32() {
    float32_t result{0};

    for (int i = 0; i < 4; ++i) {
        *((uint8_t*)(&result) + i) = readByte();
    }

    return result;
}

float64_t ByteStream::readFloat64() {
    float64_t result{0};

    for (int i = 0; i < 8; ++i) {
        *((uint8_t*)(&result) + i) = readByte();
    }
    
    return result;
}

void ByteStream::seek(int offset) {
    currentByteIndex += offset;
}

void ByteStream::printHex(uint8_t byte) {
    std::cout << std::hex;
    std::cout << std::setw(2) << std::setfill('0') << (int)byte;
    std::cout << std::dec;
}

void ByteStream::printBinary(uint8_t byte) {
    std::cout << std::bitset<8>((int)byte);
    std::cout << std::dec;
}

void ByteStream::printBinary(int32_t val) {
    std::cout << std::bitset<32>((int)val);
}

void ByteStream::printBinary(uint32_t val) {
    std::cout << std::bitset<32>(val);
}

void ByteStream::printBinary(int64_t val) {
    std::cout << std::bitset<64>(val);
}

void ByteStream::printBinary(uint64_t val) {
    std::cout << std::bitset<64>(val);
}

void ByteStream::printBinary(float32_t val) {
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&val);
    for (int i = 0; i < 4; ++i) {
        printBinary(bytes[i]);
    }
}

void ByteStream::printBinary(float64_t val) {
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&val);
    for (int i = 0; i < 8; ++i) {
        printBinary(bytes[i]);
    }
}
