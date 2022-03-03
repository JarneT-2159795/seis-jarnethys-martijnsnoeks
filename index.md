## Taak Software engineering voor geïntegreerde systemen

### Deel 0: bytestream
Ter voorbereiding van de verdere taken hebben we reeds een klasse aangemaakt om de binary WebAssembly-bestanden (wasm) te kunnen interpreteren. Hieronder staan de functies opgelijst die we nu hebben geïmplementeerd. Uiteraard wordt dit later nog uitgebreid.

```c++
// Initialiseren bytestream
ByteStream(std::string filepath);
ByteStream();
void readFile(std::string filepath);

// Byte uitlezen
std::pair<uint8_t, bool> readByte();

// ASCII-string met gegeven lengte uitlezen
std::string readASCIIString(int length);

// Verschillende types getallen uitlezen
int32_t  readInt32();
int64_t readInt64();
uint32_t readUInt32();
uint64_t readUInt64();
_Float32 readFloat32();
_Float64 readFloat64();

// Positie in bytestream aanpassen
void seek(int offset);

// Informatie over bytestream
int getCurrentByteIndex();
int getRemainingByteCount();
int getTotalByteCount();

// Bytes en getallen in verschillende formaten uitprinten
void printHex(uint8_t byte);
void printBinary(uint8_t byte);
void printBinary(int32_t val);
void printBinary(uint32_t val);
void printBinary(int64_t val);
void printBinary(uint64_t val);
```
