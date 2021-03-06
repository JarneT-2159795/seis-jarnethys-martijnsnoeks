#include <fstream>
#include "bytestream.h"
#include <iostream>
#include <iomanip>
#include <bitset>
#include <string.h>
#include <stdexcept>
#include <sstream>

ByteStream::ByteStream(std::string filepath) : currentByteIndex{ 0 } {
    if (filepath.find(".wat") != std::string::npos) {
        std::ifstream f(filepath);
        std::stringstream ss;
        ss << f.rdbuf();
        std::string file(ss.str());
        buffer = std::vector<uint8_t>(file.begin(), file.end());
        size = buffer.size();
    } else if (filepath.find(".wasm") != std::string::npos) {
        std::ifstream f(filepath, std::ios::binary);
        if (f.is_open()) {
            // Get length of file:
            f.seekg (0, f.end);
            size = f.tellg();
            f.seekg (0, f.beg);

            buffer.reserve(size);
            f.read((char*)buffer.data(), size);
        }
        f.close();
    } else {
        throw std::invalid_argument("Unsupported file: " + filepath);
    }
}

ByteStream::ByteStream(std::vector<uint8_t> stream) : currentByteIndex{ 0 } {
    size = stream.size();
    buffer = stream;
}

ByteStream::ByteStream(uint8_t *data, int size) : currentByteIndex{ 0 } {
    this->size = size;
    buffer.reserve(size);
    for (int i = 0; i < size; i++) {
        buffer.push_back(data[i]);
    }
}

ByteStream::~ByteStream() {
}

void ByteStream::addFromByteStream(ByteStream *stream) {
    buffer.insert(buffer.end(), stream->buffer.begin(), stream->buffer.end());
}

void ByteStream::readFile(std::string filepath) {
    buffer.clear();
    currentByteIndex = 0;
    std::ifstream f(filepath, std::ios::binary);
    if (f.is_open()) {
        // Get length of file:
        f.seekg (0, f.end);
        size = f.tellg();
        f.seekg (0, f.beg);

        buffer.reserve(size);
        f.read((char*)buffer.data(), size);
    }
    f.close();
}

void ByteStream::readCharVector(std::vector<uint8_t> vector) {
    buffer.clear();
    currentByteIndex = 0;
    size = vector.size();
    buffer = vector;
}

void ByteStream::readVector(std::vector<uint8_t> vector) {
    buffer.clear();
    currentByteIndex = 0;
    size = vector.size();
    buffer = vector;
}

void ByteStream::writeFile(std::string filepath) {
    std::ofstream f(filepath, std::ios::binary | std::ios::out);
    if (f.is_open()) {
        f.write((char*)buffer.data(), sizeof(uint8_t) * buffer.size());
    }
}

uint8_t ByteStream::readByte() {
    if (atEnd()) {
        throw std::out_of_range("ByteStream at end of stream");
    }
    ++currentByteIndex;
    return buffer[currentByteIndex - 1];
}

std::vector<uint8_t> ByteStream::readBytes(int amount) {
    std::vector<uint8_t> bytes;
    bytes.reserve(amount);
    for (int i = 0; i < amount; ++i) {
        bytes.push_back(readByte());
    }
    return bytes;
}

uint8_t ByteStream::peekByte() {
    return buffer[currentByteIndex];
}

void ByteStream::setByteIndex(int index) {
    currentByteIndex = index;
}

void ByteStream::writeByte(uint8_t byte){
    // TODO: error handling!
    buffer.push_back(byte);
    currentByteIndex++;
    size++;
}

void ByteStream::writeUInt32(uint32_t val) {
    do {
        unsigned char byte = val & 0x7f;
        val >>= 7;

        if (val != 0)
            byte |= 0x80;  // mark this byte to show that more bytes will follow

        writeByte(byte);
    } while (val != 0);
}

std::string ByteStream::readASCIIString(int length) {
    std::string s;

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
