#include <cstdint>
#include <string>
#include <vector>
#include <utility>

#define float32_t float
#define float64_t double

class ByteStream {
private:
    uint8_t *buffer;
    int currentByteIndex;
    int size;

public:
    ByteStream(std::string filepath);
    ByteStream(std::vector<uint8_t> stream);
    ByteStream() : currentByteIndex{0}, buffer{ nullptr } {};
    ~ByteStream();

    void readFile(std::string filepath);
    
    uint8_t readByte();
    std::vector<uint8_t> readBytes(int amount);
    bool atEnd() { return !(currentByteIndex < size); };

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
