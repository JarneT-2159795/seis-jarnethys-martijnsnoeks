# Software Engineering for Integrated Systems

# Contents

1. [Reading a binary WebAssembly file](#Reading-a-binary-WebAssembly-file)

# Reading a binary WebAssembly file

## Basics: reading a binary file in C++

The following code reads the contents of a binary file one byte at a time and stores these in a std::vector. The bytes can be read using the readByte-function. This will return the next byte in the stream or a out of range-exception when there are no more bytes. The full code can be found on [Github in the ByteStream class](https://github.com/JarneT-2159795/seis-jarnethys-martijnsnoeks/blob/main/includes/bytestream.cpp).

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

Integers in WebAssembly are Little Endian Base 128 (LEB128) encoded. This is a way to store integers with a variable amount of bits, thus using less memory. For more information you can visit [Wikipedia](https://en.wikipedia.org/wiki/LEB128).

Before decoding the integers it's important to understand how they are encoded. We'll explain this with the number 147258 which is represented as BAFE08 in LEB128. The steps for encoding an unsigned integer using two's complement are as follows:

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

## Structure of a binary WebAssembly file

The following code will read the contents of a binary file and parse it so the functions can be used in C++. A binary file consists of different sections, each describing a different part of the code. Each section needs to be parsed and the information stored in the right function so it can be used later when we want to call a function. The full code for this part can be found on [Github in the Module class](https://github.com/JarneT-2159795/seis-jarnethys-martijnsnoeks/blob/main/includes/module.cpp). For the following examples we will use the following WebAssembly code:

```javascript

(module
  (func (export "addTwo") (param i32 i32) (result i32)
    local.get 0
    local.get 1
    i32.add
  )
)

```

We can use the build output to see how each section is represented in the final binary file. Here is the output for the code above:

```

0000000: 0061 736d                                 ; WASM_BINARY_MAGIC
0000004: 0100 0000                                 ; WASM_BINARY_VERSION
; section "Type" (1)
0000008: 01                                        ; section code
0000009: 00                                        ; section size (guess)
000000a: 01                                        ; num types
; func type 0
000000b: 60                                        ; func
000000c: 02                                        ; num params
000000d: 7f                                        ; i32
000000e: 7f                                        ; i32
000000f: 01                                        ; num results
0000010: 7f                                        ; i32
0000009: 07                                        ; FIXUP section size
; section "Function" (3)
0000011: 03                                        ; section code
0000012: 00                                        ; section size (guess)
0000013: 01                                        ; num functions
0000014: 00                                        ; function 0 signature index
0000012: 02                                        ; FIXUP section size
; section "Export" (7)
0000015: 07                                        ; section code
0000016: 00                                        ; section size (guess)
0000017: 01                                        ; num exports
0000018: 06                                        ; string length
0000019: 6164 6454 776f                           addTwo  ; export name
000001f: 00                                        ; export kind
0000020: 00                                        ; export func index
0000016: 0a                                        ; FIXUP section size
; section "Code" (10)
0000021: 0a                                        ; section code
0000022: 00                                        ; section size (guess)
0000023: 01                                        ; num functions
; function body 0
0000024: 00                                        ; func body size (guess)
0000025: 00                                        ; local decl count
0000026: 20                                        ; local.get
0000027: 00                                        ; local index
0000028: 20                                        ; local.get
0000029: 01                                        ; local index
000002a: 6a                                        ; i32.add
000002b: 0b                                        ; end
0000024: 07                                        ; FIXUP func body size
0000022: 09                                        ; FIXUP section size

```

### Type section

This section stores information about function types. These function types indicate which parameters the function expects and which result types the function generates. As you can see below the type section is relatively simple to decode. The section code 0x1 is followed by the size of the section in bytes (this is standard for every section) and the amount of function types in the file. When decoding the file we have to be careful as multiple function with the same function type will only be written once. This means that the number of function types doesn't always equal the number of functions.

After all of this there will be a sequence of function types. Each type can be recognized by the 0x60 byte in front. This byte is followed by a byte indicating the number of parameters and a byte per parameter indicting the type. Possible parameter types are shown in the table below. Next a byte indicates the number of result types followed by a byte for every result type just like the parameter bytes earlier.
|Byte|Type|
|----|----|
|0x7F|i32|
|0x7E|i64|
|0x7D|f32|
|0x7C|f64|

```

; section "Type" (1)
0000008: 01                                        ; section code
0000009: 00                                        ; section size (guess)
000000a: 01                                        ; num types
; func type 0
000000b: 60                                        ; func
000000c: 02                                        ; num params
000000d: 7f                                        ; i32
000000e: 7f                                        ; i32
000000f: 01                                        ; num results
0000010: 7f                                        ; i32
0000009: 07                                        ; FIXUP section size

```

As this is a relatively simple section the code to read this section is simple too. We read in a while loop as long as we find a 0x60 byte indicating that a function type is present. When another byte is found we set the reading index of the bytestream at the previous position as this is another section. When reading a function type we first save the number of parameters. Next we add the each of the parameters to a vector so we can use them later on. We can't save them to a function yet because we haven't read the functions at this point.

```c++

void Module::readTypeSection(int length) {
    while(bytestr.readByte() == 0x60) { // 0x60 indicates a function type
        int numParams = bytestr.readByte(); // Read the amount of parameters
        std::vector<VariableType> params;
        for (int i = 0; i < numParams; ++i) {
            params.push_back(getVarType(bytestr.readByte())); // Store the type of the parameter
        }

        int numResults = bytestr.readByte(); // Read the amount of results
        std::vector<VariableType> results;
        for (int i = 0; i < numResults; ++i) {
            results.push_back(getVarType(bytestr.readByte())); // Store the type of the result
        }
        functionTypes.push_back(params);
        functionTypes.push_back(results);
    }
    bytestr.seek(-1);
}

```

### Function section

This is a very easy section where we just create a placeholder for the functions as the actual code is added at a later stage. This sections has just the number of functions and the function type of each function which we extracted earlier. As we store the parameters and results in one vector right behind eachother we multiply the index by two. You can see we also pass a lot of other variables in the constructor of the function. These will be explained later.

```

; section "Function" (3)
0000011: 03                                        ; section code
0000012: 00                                        ; section size (guess)
0000013: 01                                        ; num functions
0000014: 00                                        ; function 0 signature index
0000012: 02                                        ; FIXUP section size

```

```c++

void Module::readFunctionSection(int length) {
    int numFunctions = bytestr.readByte(); // The amount of functions in the module
    for (int i = 0; i < numFunctions; ++i) {
        // Read the type of the function
        int signature = 2 * bytestr.readByte(); // We store the parameters and results behind each other so we multiply by two
        functions.emplace_back(Function(functionTypes[signature], functionTypes[signature + 1], 
                                        &stack, &functions, &globals, &memories)); // Add the function
    }
}

```

### Memory section

```c++

void Module::readMemorySection(int length) {
    int numMemories = bytestr.readByte();
    for (int i = 0; i < numMemories; ++i) {
        if (bytestr.readByte()) {
            uint32_t initial = bytestr.readUInt32();
            uint32_t maximum = bytestr.readUInt32();
            memories.emplace_back(Memory(initial, maximum));
        } else {
            uint32_t initial = bytestr.readUInt32();
            memories.emplace_back(Memory(initial));
        }
    }
}

```

### Global section

```c++

void Module::readGlobalSection(int length) {
    int numGlobals = bytestr.readByte();
    for (int i = 0; i < numGlobals; ++i) {
        bytestr.seek(1); // skip the type
        bool isConst = bytestr.readByte() == 0;
        switch (bytestr.readByte()) {
            case I32CONST:
                globals.emplace_back(GlobalVariable(Variable(bytestr.readInt32()), isConst));
                break;
            case I64CONST:
                globals.emplace_back(GlobalVariable(Variable(bytestr.readInt64()), isConst));
                break;
            case F32CONST:
                globals.emplace_back(GlobalVariable(Variable(bytestr.readFloat32()), isConst));
                break;
            case F64CONST:
                globals.emplace_back(GlobalVariable(Variable(bytestr.readFloat64()), isConst));
                break;
            default:
                throw ModuleException("Invalid file: not a valid global type", bytestr.getCurrentByteIndex());
        }
        bytestr.seek(1); // skip the end of the global
    }
}

```

### Export section



```

; section "Export" (7)
0000015: 07                                        ; section code
0000016: 00                                        ; section size (guess)
0000017: 01                                        ; num exports
0000018: 06                                        ; string length
0000019: 6164 6454 776f                           addTwo  ; export name
000001f: 00                                        ; export kind
0000020: 00                                        ; export func index
0000016: 0a                                        ; FIXUP section size

```

```c++

void Module::readExportSection(int length) {
    int exports = bytestr.readByte();
    for (int i = 0; i < exports; ++i) {
        auto name = bytestr.readASCIIString(bytestr.readByte());
        uint8_t kind = bytestr.readByte();
        switch (kind) {
            case 0x00: // function
                functions[bytestr.readUInt32()].setName(name);
                break;
            case 0x02: // memory
                memories[bytestr.readUInt32()].setName(name);
                break;
            default:
                throw ModuleException("Invalid file: not a valid export kind", bytestr.getCurrentByteIndex());
        }
    }
}

```

### Code section

The code section contains the actual instructions for the program to function. First we read the amount of functions in the module. For each function we save the size of the body with the instructions. This section also contains the local variables for each function. We save these now so it's already done when we want to run the function. The amount of local variables is removed from the function body. For each local variable we read it's type and save it in the function. We first read how many variables of a given type are in the function. After this we read the type of the local variable and add the type and amount to the function.

```c++

void Module::readCodeSection(int length) {
    int numFunctions = bytestr.readByte(); // Amount of functions
    for (int i = 0; i < numFunctions; ++i) {
        // Amount of bytes for the body of the function
        int bodySize = bytestr.readByte() - 1;  // -1 for localVarType
        int localVarTypes = bytestr.readByte(); // Amount of local variables
        for (int j = 0; j < localVarTypes; ++j) {
            int typeCount = bytestr.readByte(); // Read the amount of variables from this type
            functions[i].addLocalVars(getVarType(bytestr.readByte()), typeCount); // Add the type and the amount of this type to the function
            bodySize -= 2;  // -2 for every local variable
        }

        functions[i].setBody(bytestr.readBytes(bodySize)); // The remaining bytes are the actual instructions
    }
}

```
