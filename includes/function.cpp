#include "function.h"
#include <cstdarg>
#include <iostream>

Function::Function(std::vector<Variable> paramaterList, std::vector<Variable> resultList, std::stack<Variable> *s) 
        : params{ paramaterList }, results{ resultList }, st{ s } {
};

void Function::setName(std::string functionName) {
    name = functionName;
}

std::string Function::getName() {
    return name;
}

void Function::addLocalVars(VariableType varType, int count) {
    for (int i = 0; i < count; ++i) {
        localVars.push_back(Variable(varType));
    }
}

void Function::setBody(std::vector<uint8_t> functionBody) {
    body = functionBody;
}

void Function::operator()(std::vector<Variable> locals) {
    std::vector<Variable> localStack = locals;
    ByteStream bs(body);
    uint8_t byte;
    while (!bs.atEnd()) {
        byte = bs.readByte();
        switch (byte)
        {
        case 0x20:
            st->push(localStack[bs.readUInt32()]);
            break;
        case 0x21:
            localStack[bs.readUInt32()] = st->top();
            break;
        case 0x41:
            st->push(Variable(VariableType::is_int32, bs.readInt32()));
            break;
        case 0x42:
            st->push(Variable(VariableType::is_int64, bs.readInt64()));
            break;
        case 0x43:
            st->push(Variable(VariableType::is_float32, bs.readFloat32()));
            break;
        case 0x44:
            st->push(Variable(VariableType::is_float64, bs.readFloat64()));
            break;
        case 0x6A:
            {
                int32_t var1 = st->top().get();
                st->pop();
                int32_t var2 = st->top().get();
                st->pop();
                st->push(Variable(VariableType::is_int32, var1 + var2));
                break;
            }
        case 0x0B:
            return;
        
        default:
            throw FunctionException("Invalid or unsupported instuction", byte);
            break;
        }
    }
}
