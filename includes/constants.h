#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

namespace constants {

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
const uint8_t IF = 0x4;
const uint8_t ELSE = 0x5;
const uint8_t CALL = 0x10;
const uint8_t BR = 0xC;
const uint8_t BR_IF = 0xD;

// Parametric instructions
const uint8_t DROP = 0x1A;
const uint8_t SELECT = 0x1B;

// Variable instructions
const uint8_t LOCALGET = 0x20;
const uint8_t LOCALSET = 0x21;
const uint8_t LOCALTEE = 0x22;
const uint8_t GLOBALGET = 0x23;
const uint8_t GLOBALSET = 0x24;

// Memory instructions
const uint8_t I32LOAD = 0x28;
const uint8_t I64LOAD = 0x29;
const uint8_t F32LOAD = 0x2A;
const uint8_t F64LOAD = 0x2B;
const uint8_t I32STORE = 0x36;
const uint8_t I64STORE = 0x37;
const uint8_t F32STORE = 0x38;
const uint8_t F64STORE = 0x39;

// Numeric instructions
const uint8_t I32CONST = 0x41;
const uint8_t I64CONST = 0x42;
const uint8_t F32CONST = 0x43;
const uint8_t F64CONST = 0x44;

// i32 comparison instructions
const uint8_t I32EQZ = 0x45;
const uint8_t I32EQ = 0x46;
const uint8_t I32NE = 0x47;
const uint8_t I32LT_S = 0x48;
const uint8_t I32LT_U = 0x49;
const uint8_t I32GT_S = 0x4A;
const uint8_t I32GT_U = 0x4B;
const uint8_t I32LE_S = 0x4C;
const uint8_t I32LE_U = 0x4D;
const uint8_t I32GE_S = 0x4E;
const uint8_t I32GE_U = 0x4F;

const uint8_t F64LT = 0x63;

const uint8_t I32CLZ = 0x67;
const uint8_t I32CTZ = 0x68;
const uint8_t I32POPCNT = 0x69;
const uint8_t I32ADD = 0x6A;
const uint8_t I32SUB = 0x6B;
const uint8_t I32MUL = 0x6C;
const uint8_t I32DIV_S = 0x6D;
const uint8_t I32DIV_U = 0x6E;
const uint8_t I32REM_S = 0x6F;
const uint8_t I32REM_U = 0x70;
const uint8_t I32AND = 0x71;
const uint8_t I32OR = 0x72;
const uint8_t I32XOR = 0x73;
const uint8_t I32SHL = 0x74;
const uint8_t I32SHR_S = 0x75;
const uint8_t I32SHR_U = 0x76;
const uint8_t I32ROTL = 0x77;
const uint8_t I32ROTR = 0x78;

const uint8_t I64ADD = 0x7C;
const uint8_t I64SUB = 0x7D;
const uint8_t I64MUL = 0x7E;

const uint8_t F32ADD = 0x92;
const uint8_t F32SUB = 0x93;
const uint8_t F32MUL = 0x94;

const uint8_t F64ADD = 0xA0;
const uint8_t F64SUB = 0xA1;
const uint8_t F64MUL = 0xA2;

// Numeric conversion instructions
// to i32
const uint8_t I32WRAP_I64 = 0xA7;
const uint8_t I32TRUNC_F32_S = 0xA8;
const uint8_t I32TRUNC_F64_S = 0xAA;
const uint8_t I32REINTERPRET_F32 = 0xBC;
// to i64
const uint8_t I64TRUNC_F32_S = 0xAE;
const uint8_t I64TRUNC_F64_S = 0xB0;
const uint8_t I64REINTERPRET_F64 = 0xBD;

// Other instructions
const uint8_t BLOCK_END = 0x0B;
const uint8_t MEMORY_BULK_OP = 0xFC;

}

#endif
