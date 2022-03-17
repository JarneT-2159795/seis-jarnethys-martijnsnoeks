#include <cstdint>

const uint8_t LOCALGET = 0x20;
const uint8_t LOCALSET = 0x21;

const uint8_t I32CONST = 0x41;
const uint8_t I64CONST = 0x42;
const uint8_t F32CONST = 0x43;
const uint8_t F64CONST = 0x44;

const uint8_t I32ADD = 0x6A;
const uint8_t I32SUB = 0x6B;
const uint8_t I32MUL = 0x6C;

const uint8_t I64ADD = 0x7C;
const uint8_t I64SUB = 0x7D;
const uint8_t I64MUL = 0x7E;

const uint8_t F32ADD = 0x92;
const uint8_t F32SUB = 0x93;
const uint8_t F32MUL = 0x94;

const uint8_t F64ADD = 0xA0;
const uint8_t F64SUB = 0xA1;
const uint8_t F64MUL = 0xA2;

const uint8_t END = 0x0B;
