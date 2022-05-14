#ifndef _AST_FUNCTION_H_
#define _AST_FUNCTION_H_

#include <vector>
#include <string>
#include "instruction.h"
#include "variabletype.h"

typedef struct AST_Function {
    std::string name;
    std::vector<VariableType> parameters = {};
    std::vector<VariableType> results = {};
    std::vector<Instruction*>* body = nullptr;
} AST_Function;


#endif
