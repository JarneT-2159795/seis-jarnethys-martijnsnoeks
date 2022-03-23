#include <cstdint>

// File sections
const uint8_t CUSTOM_SECTION = 0x0;
const uint8_t TYPE_SECTION = 0x1;
const uint8_t IMPORT_SECTION = 0x2;
const uint8_t FUNCTION_SECTION = 0x3;
const uint8_t TABLE_SECTION = 0x4;
const uint8_t MEMORY_SECTION = 0x5;
const uint8_t GLOBAL_SECTION = 0x6;
const uint8_t EXPORT_SECTION = 0x7;
const uint8_t START_SECTION = 0x8;
const uint8_t ELEMENT_SECTION = 0x9;
const uint8_t CODE_SECTION = 0xA;
const uint8_t DATA_SECTION = 0xB;
const uint8_t DATACOUNT_SECTION = 0xC;

// Variable types
const uint8_t INT32 = 0x7F;
const uint8_t INT64 = 0x7E;
const uint8_t FLOAT32 = 0x7D;
const uint8_t FLOAT64 = 0x7C;

// Control instructions
const uint8_t BLOCK = 0x2;
const uint8_t CALL = 0x10;
const uint8_t BR = 0xC;
const uint8_t BR_IF = 0xD;

// Variable instructions
const uint8_t LOCALGET = 0x20;
const uint8_t LOCALSET = 0x21;

// Numeric instructions
const uint8_t I32CONST = 0x41;
const uint8_t I64CONST = 0x42;
const uint8_t F32CONST = 0x43;
const uint8_t F64CONST = 0x44;

const uint8_t I32EQZ = 0x45;
const uint8_t I32EQ = 0x46;
const uint8_t I32GT_S = 0x4A;

const uint8_t F64LT = 0x63;

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

// Expressions
const uint8_t END = 0x0B;
