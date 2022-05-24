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

        if ( name == "i32.add" ) return constants::I32ADD;
        if ( name == "i32.sub" ) return constants::I32SUB;
        if ( name == "i32.mul" ) return constants::I32MUL;
        if ( name == "i32.const" ) return constants::I32CONST;
        if ( name == "i32.eq" ) return constants::I32EQ;

        if (name == "f32.add") return constants::F32ADD;
        if (name == "f32.sub") return constants::F32SUB;
        if (name == "f32.mul") return constants::F32MUL;

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
                constants::BLOCK_END, constants::IF, constants::ELSE, constants::CALL
        };
        for (uint8_t noParamOP : noParamOPs) {
            if ( noParamOP == op ) return true;
        }
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
};
#endif // __INSTRUCTION_H__