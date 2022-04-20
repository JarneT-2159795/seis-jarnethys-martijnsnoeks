// Based on example on Toledo

#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include <cstdint>
#include <string>

// shamelessly inspired by https://github.com/TimGysen/GysenVanherckSEIS/blob/main/VM/VM/Instruction.h
class InstructionNumber {

public:
    enum class Section { TYPE=0x01, IMPORT=0x02, FUNCTION=0X03, TABLE=0x04, MEMORY=0x05, GLOBAL=0x06, EXPORT=0x07, START=0x08, ELEMENT=0x09, CODE=0x0a, DATA=0x0b, CUSTOM=0x00 };
    enum class Type { NONE=0x00, I32=0x7f, I64=0x7e, F32=0x7d, F64=0x7c, FUNC=0x60 };
    enum class Operation{ NONE=0x00, LOCALGET=0x20, LOCALSET=0x21, I32ADD=0x6a, I32SUB = 0x6b, I32MUL=0x6c, I32DIV=0x6d , F32ADD=0x92 , F32SUB = 0x93, F32MUL = 0x94, END=0x0b, CALL=0x10,
                          I32CONST=0x41, I32EQ=0x46, IF=0x04, ELSE=0x05 };

    static Operation getOperation(std::string name) {
        if ( name == "local.get" ) return Operation::LOCALGET;
        if ( name == "local.set" ) return Operation::LOCALSET;
        if ( name == "i32.add" ) return Operation::I32ADD;
        if ( name == "i32.sub" ) return Operation::I32SUB;
        if ( name == "i32.const" ) return Operation::I32CONST;
        if ( name == "i32.eq" ) return Operation::I32EQ;
        if ( name == "if" ) return Operation::IF;
        if ( name == "else" ) return Operation::ELSE;
        if ( name == "call" ) return Operation::CALL;
        if ( name == "end" ) return Operation::END;
        // TODO: support other operations!

        return Operation::NONE;
    }

    static Type getType(std::string name) {
        if ( name == "i32" ) return Type::I32;
        // TODO: support other types!

        return Type::NONE;
    }

    static bool isCalculation( Operation op ) {
        return op == Operation::I32ADD; // TODO: allow more calculations than just i32.add!
    }

    static bool isConst( Operation op ) {
        return op == Operation::I32CONST; // TODO: allow more consts than just i32
    }

    static bool hasParameter( Operation op ) {
        return !InstructionNumber::hasNoParameter(op);
    }

    static bool hasNoParameter( Operation op ) {
        return 
            op == Operation::IF ||
            op == Operation::ELSE ||
            op == Operation::END ||
            op == Operation::I32EQ ||
            op == Operation::NONE;
    }
};

// TODO: add proper types that can be used for more than just a single compiler optimization!
// CALCULATION = takes 2 things from the stack and puts result on stack (e.g., i32.add, i32.eq, etc.)
// CONST = places a constant value on the stack (e.g., i32.const 15)
// INSTRUCTION_WITH_PARAMETER = any other instruction that needs to be serialized with its parameter (e.g., local.get)
// INSTRUCTION_WITHOUT_PARAMETER = any other instruction that needs to be serialized without its parameter (e.g., end, i32, if, else )
enum class InstructionType { CALCULATION = 10, CONST = 20, INSTRUCTION_WITH_PARAMETER = 30, INSTRUCTION_WITHOUT_PARAMETER = 40 };

class Instruction
{
public:
    Instruction(InstructionType type) : type(type) {}

    ~Instruction(){};

    InstructionType type;

    uint32_t instruction_code = (uint32_t) InstructionNumber::Operation::NONE;
    uint32_t parameter = 0;
};
#endif // __INSTRUCTION_H__