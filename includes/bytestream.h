#include <cstdint>
#include <string>
#include <vector>
#include <utility>

class ByteStream {
private:
    std::vector<uint8_t> buffer;
    int currentByteIndex;

public:
    ByteStream(std::string filepath);
    ByteStream() : currentByteIndex{0}{};

    void readFile(std::string filepath);
    
    uint8_t readByte();
    bool atEnd() { return (currentByteIndex > buffer.size()); };

    std::string readASCIIString(int length);

    int32_t  readInt32();
    int64_t readInt64();

    uint32_t readUInt32();
    uint64_t readUInt64();

    _Float32 readFloat32();
    _Float64 readFloat64();

    void seek(int offset);

    int getCurrentByteIndex() { return currentByteIndex; };
    int getRemainingByteCount() { return buffer.size() - currentByteIndex; };
    int getTotalByteCount() { return buffer.size(); };
    void printHex(uint8_t byte);
    void printBinary(uint8_t byte);
    void printBinary(int32_t val);
    void printBinary(uint32_t val);
    void printBinary(int64_t val);
    void printBinary(uint64_t val);
    void printBinary(_Float32 val);
    void printBinary(_Float64 val);
};
