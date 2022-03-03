#include <fstream>
#include "bytestream.h"
#include <iostream>
#include <iomanip>
#include <bitset>

ByteStream::ByteStream(std::string filepath) : currentByteIndex{ 0 } {
    // https://stackoverflow.com/questions/11162608/reading-bytes-with-ifstream
    std::ifstream f(filepath, std::ios::binary);
    if (f.is_open()) {
        char c;
        while (!f.eof()) {
            f.read(&c, 1);
            buffer.push_back((uint8_t)c);
        }
    }
    f.close();
    buffer.pop_back();
}

void ByteStream::readFile(std::string filepath) {
    buffer.clear();
    currentByteIndex = 0;
    std::ifstream f(filepath, std::ios::binary);
    if (f.is_open()) {
        char c;
        while (!f.eof()) {
            f.read(&c, 1);
            buffer.push_back((uint8_t)c);
        }
    }
    f.close();
    buffer.pop_back();
}

std::pair<uint8_t, bool> ByteStream::readByte() {
    if (currentByteIndex < buffer.size()) {
        ++currentByteIndex;
        return std::make_pair(buffer[currentByteIndex - 1], true);
    }
    return std::make_pair(' ', false);
}

std::string ByteStream::readASCIIString(int length) {
    std::string s{""};
    for (int i = 0; i < length; ++i) {
        s += readByte().first;
    }
    return s;
}

int32_t ByteStream::readInt32() {
    // https://en.wikipedia.org/wiki/LEB128
    int32_t result{0};
    int shift{0};
    uint8_t byte;
    do
    {
        byte = readByte().first;
        result |= ((byte & 0b0111'1111) << shift);
        shift += 7;
    } while ((byte & 0b1000'0000) != 0);

    if ((shift < 32) && ((byte & 0b0100'0000) == 0x40)) {
        result |= (~0 << shift);
    }
    return result;
}

int64_t ByteStream::readInt64() {
    // https://en.wikipedia.org/wiki/LEB128
    // https://stackoverflow.com/questions/30777450/bitwise-bitshift-operations-on-64-bit-integers-in-c
    int64_t result{};
    int shift{0};
    uint8_t byte;
    do
    {
        byte = readByte().first;
        result |= ((int64_t)(byte & 127) << shift);
        shift += 7;
    } while ((byte & 128) != 0);
    if ((shift < 64) && ((byte & 64) == 64)) {
        result |= (~0 << shift);
    }
    return result;
}

uint32_t ByteStream::readUInt32() {
    // https://en.wikipedia.org/wiki/LEB128
    uint32_t result{0};
    int shift{0};

    while (true) {
        uint8_t byte = readByte().first;
        result |= ((byte & 0b0111'1111) << shift);
        if ((byte & 0b1000'0000) == 0) {
            break;
        }
        shift +=7;
    }
    return result;
}

uint64_t ByteStream::readUInt64() {
    // https://en.wikipedia.org/wiki/LEB128
    // https://stackoverflow.com/questions/30777450/bitwise-bitshift-operations-on-64-bit-integers-in-c
    uint64_t result{0};
    int shift{0};

    while (true) {
        uint8_t byte = readByte().first;
        result |= ((uint64_t)(byte & 0b0111'1111) << shift);
        if ((byte & 0b1000'0000) == 0) {
            break;
        }
        shift +=7;
    }
    return result;
}

_Float32 ByteStream::readFloat32() {
    // https://stackoverflow.com/questions/3991478/building-a-32-bit-float-out-of-its-4-composite-bytes
    _Float32 result{0};
    for (int i = 0; i < 4; ++i) {
        *((uint8_t*)(&result) + i) = readByte().first;
    }
    return result;
}

_Float64 ByteStream::readFloat64() {
    // https://stackoverflow.com/questions/3991478/building-a-32-bit-float-out-of-its-4-composite-bytes
    _Float64 result{0};
    for (int i = 0; i < 8; ++i) {
        *((uint8_t*)(&result) + i) = readByte().first;
    }
    return result;
}

void ByteStream::seek(int offset) {
    currentByteIndex += offset;
}

void ByteStream::printHex(uint8_t byte) {
    // https://stackoverflow.com/questions/673240/how-do-i-print-an-unsigned-char-as-hex-in-c-using-ostream
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
    std::cout << std::bitset<32>((int)val);
}

void ByteStream::printBinary(int64_t val) {
    std::cout << std::bitset<64>((int)val);
}

void ByteStream::printBinary(uint64_t val) {
    std::cout << std::bitset<64>((int)val);
}
