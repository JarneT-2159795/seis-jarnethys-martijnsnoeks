// Based on example on Toledo

#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include <cstdint>
#include <string>
#include "constants.h"

// shamelessly inspired by https://github.com/TimGysen/GysenVanherckSEIS/blob/main/VM/VM/Instruction.h
class InstructionNumber {

public:
    enum class Section { TYPE=0x01, IMPORT=0x02, FUNCTION=0X03, TABLE=0x04, MEMORY=0x05, GLOBAL=0x06, EXPORT=0x07, START=0x08, ELEMENT=0x09, CODE=0x0a, DATA=0x0b, CUSTOM=0x00 };
    enum class Type { NONE=0x00, I32=0x7f, I64=0x7e, F32=0x7d, F64=0x7c, FUNC=0x60 };

    static uint8_t getOperation(const std::string& name) {
        if ( name == "local.get" ) return constants::LOCALGET;
        if ( name == "local.set" ) return constants::LOCALSET;
        if ( name == "local.tee" ) return constants::LOCALTEE;

        if ( name == "i32.add" ) return constants::I32ADD;
        if ( name == "i32.sub" ) return constants::I32SUB;
        if ( name == "i32.mul" ) return constants::I32MUL;
        if ( name == "i32.div_s" ) return constants::I32DIV_S;
        if ( name == "i32.div_u" ) return constants::I32DIV_U;
        if ( name == "i32.const" ) return constants::I32CONST;

        if ( name == "i32.eqz" ) return constants::I32EQZ;
        if ( name == "i32.eq" ) return constants::I32EQ;
        if ( name == "i32.ne" ) return constants::I32NE;
        if ( name == "i32.lt_s" ) return constants::I32LT_S;
        if ( name == "i32.lt_u" ) return constants::I32LT_U;
        if ( name == "i32.gt_s" ) return constants::I32GT_S;
        if ( name == "i32.gt_u" ) return constants::I32GT_U;
        if ( name == "i32.le_s" ) return constants::I32LE_S;
        if ( name == "i32.le_u" ) return constants::I32LE_U;
        if ( name == "i32.ge_s" ) return constants::I32GE_S;
        if ( name == "i32.ge_u" ) return constants::I32GE_U;

        if (name == "i32.and") return constants::I32AND;
        if (name == "i32.or") return constants::I32OR;
        if (name == "i32.xor") return constants::I32XOR;
        if (name == "i32.shl") return constants::I32SHL;
        if (name == "i32.shr_s") return constants::I32SHR_S;
        if (name == "i32.shr_u") return constants::I32SHR_U;

        if (name == "i32.wrap_i64") return constants::I32WRAP_I64;
        if (name == "i32.trunc_f32_s") return constants::I32TRUNC_F32_S;
        if (name == "i32.reinterpret_f32") return constants::I32REINTERPRET_F32;

        if ( name == "i64.add" ) return constants::I64ADD;
        if ( name == "i64.sub" ) return constants::I64SUB;
        if ( name == "i64.mul" ) return constants::I64MUL;
        if ( name == "i64.const" ) return constants::I64CONST;

        if (name == "f32.add") return constants::F32ADD;
        if (name == "f32.sub") return constants::F32SUB;
        if (name == "f32.mul") return constants::F32MUL;
        if (name == "f32.const") return constants::F32CONST;

        if (name == "f64.add") return constants::F64ADD;
        if (name == "f64.sub") return constants::F64SUB;
        if (name == "f64.mul") return constants::F64MUL;
        if (name == "f64.const") return constants::F64CONST;

        if (name == "i32.load") return constants::I32LOAD;
        if (name == "i32.store") return constants::I32STORE;

        if (name == "memory.size") return constants::MEMORYSIZE;
        if (name == "memory.grow") return constants::MEMORYGROW;

        if ( name == "if" ) return constants::IF;
        if ( name == "else" ) return constants::ELSE;
        if ( name == "call" ) return constants::CALL;
        if ( name == "end" ) return constants::BLOCK_END;
        // TODO: support other operations!

        printf("Unsupported operation: %s\n", name.c_str());
        return 0;
    }

    static Type getType(const std::string& name) {
        if ( name == "i32" ) return Type::I32;
        if ( name == "i64" ) return Type::I64;
        if ( name == "f32" ) return Type::F32;
        if ( name == "f64" ) return Type::F64;

        return Type::NONE;
    }

    static bool isCalculation( uint8_t op ) {
        const std::vector<uint8_t> calcOPs = {
                constants::I32ADD, constants::I32SUB, constants::I32MUL,
                constants::I64ADD, constants::I64SUB, constants::I64MUL,
                constants::F32ADD, constants::F32SUB, constants::F32MUL,
                constants::F64ADD, constants::F64SUB, constants::F64MUL
        };
        for (uint8_t calcOP : calcOPs) {
            if ( calcOP == op ) return true;
        }
        return false;
    }

    static bool isConst( uint8_t op ) {
        const std::vector<uint8_t> constOPs = {
                constants::I32CONST, constants::I64CONST, constants::F32CONST, constants::F64CONST
        };
        for (uint8_t constOP : constOPs) {
            if ( constOP == op ) return true;
        }
        return false;
    }

    static bool hasParameter( uint8_t op ) {
        return !InstructionNumber::hasNoParameter(op);
    }

    static bool hasNoParameter( uint8_t op ) {
        const std::vector<uint8_t> noParamOPs = {
                constants::BLOCK_END, constants::ELSE,
                constants::I32ADD, constants::I32SUB, constants::I32MUL, constants::I32DIV_S,
                constants::I32DIV_U, constants::I32EQZ, constants::I32EQ, constants::I32NE,
                constants::I32LT_S, constants::I32LT_U, constants::I32GT_S, constants::I32GT_U,
                constants::I32LE_S, constants::I32LE_U, constants::I32GE_S, constants::I32GE_U,
                constants::I32AND, constants::I32OR, constants::I32XOR, constants::I32SHL,
                constants::I32SHR_S, constants::I32SHR_U, constants::I32WRAP_I64,
                constants::I32TRUNC_F32_S, constants::I32REINTERPRET_F32
        };
        for (uint8_t noParamOP : noParamOPs) {
            if ( noParamOP == op ) return true;
        }
        return false;
    }
};

// TODO: add proper types that can be used for more than just a single compiler optimization!
// CALCULATION = takes 2 things from the stack and puts result on stack (e.g., i32.add, i32.eq, etc.)
// CONST = places a constant value on the stack (e.g., i32.const 15)
// INSTRUCTION_WITH_PARAMETER = any other instruction that needs to be serialized with its parameter (e.g., local.get)
// INSTRUCTION_WITHOUT_PARAMETER = any other instruction that needs to be serialized without its parameter (e.g., end, i32, if, else )
enum class InstructionType { CALCULATION = 10, CONST = 20, INSTRUCTION_WITH_PARAMETER = 30, INSTRUCTION_WITHOUT_PARAMETER = 40 };

class Instruction {
public:
    Instruction(InstructionType type) : type(type) {}
    Instruction(InstructionType type, uint32_t opCode) : type(type), instruction_code(opCode) {}

    ~Instruction(){};

    InstructionType type;

    uint32_t instruction_code = (uint32_t) 0;
    uint32_t parameter = 0;
    float32_t float_parameter = 0.0;
    std::vector<uint8_t> block_parameters;
};
#endif // __INSTRUCTION_H__