#include "function.h"

Function::Function(std::vector<Variable> paramaterList, std::vector<Variable> resultList) : params{ paramaterList }, results{ resultList } {
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
