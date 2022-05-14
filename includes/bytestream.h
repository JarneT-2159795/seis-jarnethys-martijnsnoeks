#include <cstdint>
#include <string>
#include <vector>
#include <utility>

#ifndef __BYTESTREAM_H__
#define __BYTESTREAM_H__

#define float32_t float
#define float64_t double

class ByteStream {
private:
    std::vector<uint8_t> buffer;
    int currentByteIndex;
    int size;

public:
    ByteStream(std::string filepath);
    ByteStream(std::vector<uint8_t> stream);
    ByteStream(uint8_t *data, int size);
    ByteStream() : currentByteIndex(0), buffer(), size(0) {};
    ~ByteStream();

    void addFromByteStream(ByteStream *stream);
    void readFile(std::string filepath);
    void readVector(std::vector<uint8_t> vector);

    void writeFile(std::string filepath);
    
    uint8_t readByte();
    std::vector<uint8_t> readBytes(int amount);
    uint8_t peekByte();
    void setByteIndex(int index);
    bool atEnd() { return !(currentByteIndex < size); };

    void writeByte(uint8_t byte);
    void writeUInt32(uint32_t value);
    void fixUpByte(int index, uint8_t byte) { buffer[index] = byte; };

    std::string readASCIIString(int length);

    int32_t  readInt32();
    int64_t readInt64();

    uint32_t readUInt32();
    uint64_t readUInt64();

    float32_t readFloat32();
    float64_t readFloat64();

    void seek(int offset);

    int getCurrentByteIndex() { return currentByteIndex; };
    int getRemainingByteCount() { return size - currentByteIndex; };
    int getTotalByteCount() { return size; };
    void printHex(uint8_t byte);
    void printBinary(uint8_t byte);
    void printBinary(int32_t val);
    void printBinary(uint32_t val);
    void printBinary(int64_t val);
    void printBinary(uint64_t val);
    void printBinary(float32_t val);
    void printBinary(float64_t val);
};
#endif // __BYTESTREAM_H__
