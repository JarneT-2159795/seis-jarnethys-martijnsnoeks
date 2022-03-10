# Software Engineering for Integrated Systems

# Contents

1. [Reading a binary WebAssembly file](#Reading-a-binary-WebAssembly-file)

# Reading a binary WebAssembly file

## Basics: reading a binary file in C++

The following code reads the contents of a binary file one byte at a time and stores these in a std::vector. The bytes can be read using the readByte-function. This will return the next byte in the stream or a out of range-exception when there are no more bytes.

```c++

ByteStream::ByteStream(std::string filepath) {
    std::ifstream f(filepath, std::ios::binary);
    if (f.is_open()) {
        char c;
        while (!f.eof()) {
            f.read(&c, 1);
            buffer.push_back((uint8_t)c);
        }
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

```

## Parsing numbers

### Integers

#### Encoding unsigned integers

Integers in WebAssembly are Little Endian Base 128 (LEB128) encoded. This is a way to store integers with a variable amount of bits, thus using less memory. Before decoding the integers it's important to understand how they are encoded. We'll explain this with the number 147258 which is represented as BAFE08 in LEB128. The steps for encoding an unsigned integer using two's complement are as follows:

1. Convert the positive number to binary
2. Add zeros to the left so you end op with a multiple of seven bits
3. Split the binary number in groups of seven bits
4. Add ones as the most significant bit (left most bit) for every group of seven bits except the most left group. Here we add a zero
5. Convert every group to its hexadecimal value
6. Reverse the order of the groups so the right most group becomes the left most group and vice versa

```

        100011111100111010      Convert the number (147258) to binary
     000100011111100111010      Add zeros to the left so we end op with a multiple of seven bits (21)
 0001000  1111110  0111010      Split in groups of seven bits
00001000 11111110 10111010      Add 1's to all but the left most group of bits. Add a zero to the left most group
    0x08     0xFE     0xBA      Convert every group to it's hexadecimal representation
    0xBA     0xFE     0x08      Invert the order of the groups

```

#### Encoding signed integers

Signed integers require two extra steps to transform the binary number to it's two's complement.

1. Convert the positive number to binary
2. Add zeros to the left so you end op with a multiple of seven bits
3. Invert all bits
4. Add one
5. Split the binary number in groups of seven bits
6. Add ones as the most significant bit (left most bit) for every group of seven bits except the most left group. Here we add a zero
7. Convert every group to its hexadecimal value
8. Reverse the order of the groups so the right most group becomes the left most group and vice versa

```

        100011111100111010      Convert the positive number (147258) to binary
     000100011111100111010      Add zeros to the left so we end op with a multiple of seven bits (21)
     111011100000011000101      Invert all bits
     111011100000011000110      Add one
 1110111  0000001  1000110      Split in groups of seven bits
01110111 10000001 11000110      Add 1's to all but the left most group of bits. Add a zero to the left most group
    0x77     0x81     0xC6      Convert every group to it's hexadecimal representation
    0xC6     0x81     0x77      Invert the order of the groups

```

#### Decoding unsigned integers

To decode an unsigned integer we reverse the process above. The example will decode the bytes 959AEF3A to 123456789. Below is also a C++ implementation which reads the bytes using the readByte-function shown earlier.

1. Read a byte
2. Perform a bitwise or with 0111 1111 to only keep the seven bits that represent part of the number
3. Shift the bits a multiple of seven because the first byte represents the right most bits and the last byte represents the left most bits
4. Perform a bitwise or with the shifted byte and the result
5. Increase the shift variable with seven for the next shift
6. Repeat steps 1 - 5 until the left most bit is 0

```

      95       9A       EF       3A     Bytes read from bytestream
10010101 10011010 11101111 00111010     Binary representation of every byte
 0010101  0011010  1101111  0111010     Remove the left most bit from every group of bits
 0111010  1101111  0011010  0010101     Binary representation after shifting
                          123456789     Decimal representation

```

```c++

uint32_t ByteStream::readUInt32() {
    uint32_t result{0}; // The resulting number
    int shift{0};   // The amount of places to shift the bits

    while (true) {
        uint8_t byte = readByte();  // Read a byte
        result |= ((byte & 0b0111'1111) << shift);  // Shift the seven bits that represent part of the number
        if ((byte & 0b1000'0000) == 0) {    // Continue as long as the left most bit is 1
            break;
        }
        shift += 7; // Increase the number of places to shift the next part of the number
    }

    return result;
}

```

#### Decoding signed integers

Decoding a signed integer follows the same steps as for an unsigned integer except when we have read all the bytes we perform a sign extension. Sign extension increases the amount of bits of a binary number while also preserving it's sign and value. As we have a binary number represented by it's two's complement we insert bits to the left with the same value as the sign bit. When the sign bit is low we don't have to perform this operation because the bits that are not represented will be zero automatically. An important note when decoding 64 bit integers is to cast the result of the bitwise and between the byte and 0111 1111 to a 64 bit integer before performing the bitwise or with the result. Otherwise a bitwise or between a 64 and 32 bit integer will be performed resulting in wrong results. Again a C++ implementation as shown.

1. Read a byte
2. Perform a bitwise or with 0111 1111 to only keep the seven bits that represent part of the number
3. Shift the bits a multiple of seven because the first byte represents the right most bits and the last byte represents the left most bits
4. Perform a bitwise or with the shifted byte and the result
5. Increase the shift variable with seven for the next shift
6. Repeat steps 1 - 5 until the left most bit is 0

```

                 A5       A5       88       C7       88       68     Bytes read from bytestream
           10100101 10100101 10001000 11000111 10001000 01101000     Binary representation of every byte
            0100101  0100101  0001000  1000111  0001000  1101000     Remove the left most bit from every group of bits
            1101000  0001000  1000111  0001000  0100101  0100101     Binary representation after shifting
1111111111111111111111110100000010001000111000100001001010100101     Sign extension to 64 bits
                                                   -822337203547     Decimal representation

```

```c++

int64_t ByteStream::readInt64() {
    int64_t result{0};  // The resulting number
    int shift{0};   // The amount of places to shift the bits
    uint8_t byte;   // The read byte

    do
    {
        byte = readByte();  // Read a byte
        result |= ((int64_t)(byte & 0b0111'1111) << shift); // Shift the seven bits that represent part of the number
        shift += 7;     // Increase the number of places to shift the next part of the number
    } while ((byte & 0b1000'0000) != 0);    // Continue as long as the left most bit is 1

    // Check if less then 64 bits are shifted and the sign bit is high
    if ((shift < 64) && ((byte & 0b0100'0000) == 0b0100'0000)) {
        result |= (~0 << shift);    // Set all bits left of the sign bit high
    }

    return result;
}

```

### Floating-Point

Decoding a float in WebAssembly is much more straightforward in respect to the integers. As the floating point numbers are not encoded in a special format the bytes can just be read from the stream and the bits placed in their corresponding location in the resulting float. The C++ implementation can be found below.

```

      77       BE       9F       1A       2F       DD       5E       C0     4/8 bytes read for a 32/64 bit floating point
01110111 10111110 10011111 00011010 00101111 11011101 01011110 11000000     Binary representation of each byte
                                                               -123.456     Decimal representation of the floating point

```

```c++

_Float64 ByteStream::readFloat64() {
    _Float64 result{0};

    for (int i = 0; i < 8; ++i) {
        *((uint8_t*)(&result) + i) = readByte();
    }
    
    return result;
}

```

## Parsing strings

Parsing strings from bytecode is rather simple because all ASCII characters are represented by bytes anyway. Given the length of the string each byte can be read, converted to a character and appended to the final string.

```c++

std::string ByteStream::readASCIIString(int length) {
    std::string s{""};

    for (int i = 0; i < length; ++i) {
        s += readByte();
    }

    return s;
}

```
