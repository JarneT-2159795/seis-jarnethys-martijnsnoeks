#include "function.h"
#include <cstdarg>
#include <iostream>

Function::Function(std::vector<VariableType> paramaterList, std::vector<VariableType> resultList, std::vector<Variable> *globalStack) 
            : params{ paramaterList }, results{ resultList }, stack{ globalStack } {
};

void Function::setName(std::string functionName) {
    name = functionName;
}

std::string Function::getName() {
    return name;
}

void Function::addLocalVars(VariableType varType, int count) {
    for (int i = 0; i < count; ++i) {
        localVars.push_back(varType);
    }
}

void Function::setBody(std::vector<uint8_t> functionBody) {
    body = functionBody;
}

void Function::operator()(int offset) {
    stackOffset = offset;
    for (auto par : localVars) {
        switch (par) {
            case VariableType::is_int32:
                stack->push_back(int32_t());
                break;
            case VariableType::is_int64:
                stack->push_back(int64_t());
                break;
            case VariableType::isfloat32_t:
                stack->push_back(float32_t());
                break;
            case VariableType::isfloat64_t:
                stack->push_back(float64_t());
                break;
        }
    }
    ByteStream bs(body);
    uint8_t byte;
    while (!bs.atEnd()) {
        byte = bs.readByte();
        switch (byte)
        {
        case LOCALGET:
            stack->push_back(stack->at(bs.readUInt32() + offset));
            break;
        case LOCALSET:
            stack->at(bs.readUInt32() + offset) = stack->back();
            break;
        case I32CONST:
            stack->push_back(int32_t(bs.readInt32()));
            break;
        case I64CONST:
            stack->push_back(int64_t(bs.readInt64()));
            break;
        case F32CONST:
            stack->push_back(float32_t(bs.readFloat32()));
            break;
        case F64CONST:
            stack->push_back(float64_t(bs.readFloat64()));
            break;
        case I32ADD:
            {
                int32_t var1 = std::get<int32_t>(stack->back());
                stack->pop_back();
                int32_t var2 = std::get<int32_t>(stack->back());
                stack->pop_back();
                stack->push_back(int32_t(var1 + var2));
                break;
            }
        case I32SUB:
            {
                int32_t var1 = std::get<int32_t>(stack->back());
                stack->pop_back();
                int32_t var2 = std::get<int32_t>(stack->back());
                stack->pop_back();
                stack->push_back(int32_t(var1 - var2));
                break;
            }
        case I32MUL:
            {
                int32_t var1 = std::get<int32_t>(stack->back());
                stack->pop_back();
                int32_t var2 = std::get<int32_t>(stack->back());
                stack->pop_back();
                stack->push_back(int32_t(var1 * var2));
                break;
            }
        case I64ADD:
            {
                int64_t var1 = std::get<int64_t>(stack->back());
                stack->pop_back();
                int64_t var2 = std::get<int64_t>(stack->back());
                stack->pop_back();
                stack->push_back(int64_t(var1 + var2));
                break;
            }
        case I64SUB:
            {
                int64_t var1 = std::get<int64_t>(stack->back());
                stack->pop_back();
                int64_t var2 = std::get<int64_t>(stack->back());
                stack->pop_back();
                stack->push_back(int64_t(var1 - var2));
                break;
            }
        case I64MUL:
            {
                int64_t var1 = std::get<int64_t>(stack->back());
                stack->pop_back();
                int64_t var2 = std::get<int64_t>(stack->back());
                stack->pop_back();
                stack->push_back(int64_t(var1 * var2));
                break;
            }
        case F32ADD:
            {
                float32_t var1 = std::get<float32_t>(stack->back());
                stack->pop_back();
                float32_t var2 = std::get<float32_t>(stack->back());
                stack->pop_back();
                stack->push_back(float32_t(var1 + var2));
                break;
            }
        case F32SUB:
            {
                float32_t var1 = std::get<float32_t>(stack->back());
                stack->pop_back();
                float32_t var2 = std::get<float32_t>(stack->back());
                stack->pop_back();
                stack->push_back(float32_t(var1 - var2));
                break;
            }
        case F32MUL:
            {
                float32_t var1 = std::get<float32_t>(stack->back());
                stack->pop_back();
                float32_t var2 = std::get<float32_t>(stack->back());
                stack->pop_back();
                stack->push_back(float32_t(var1 * var2));
                break;
            }
        case F64ADD:
            {
                float64_t var1 = std::get<float64_t>(stack->back());
                stack->pop_back();
                float64_t var2 = std::get<float64_t>(stack->back());
                stack->pop_back();
                stack->push_back(float64_t(var1 + var2));
                break;
            }
        case F64SUB:
            {
                float64_t var1 = std::get<float64_t>(stack->back());
                stack->pop_back();
                float64_t var2 = std::get<float64_t>(stack->back());
                stack->pop_back();
                stack->push_back(float64_t(var1 + var2));
                break;
            }
        case F64MUL:
            {
                float64_t var1 = std::get<float64_t>(stack->back());
                stack->pop_back();
                float64_t var2 = std::get<float64_t>(stack->back());
                stack->pop_back();
                stack->push_back(float64_t(var1 + var2));
                break;
            }
        case END:
            return;
        
        default:
            throw FunctionException("Invalid or unsupported instuction", byte);
            break;
        }
    }
}
