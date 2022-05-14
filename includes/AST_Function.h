#ifndef _AST_FUNCTION_H_
#define _AST_FUNCTION_H_

#include <vector>
#include <string>
#include "instruction.h"

enum class VariableType { is_int32, is_int64, isfloat32_t, isfloat64_t };

typedef struct AST_Function {
    std::string name;
    std::vector<VariableType> parameters = {};
    std::vector<VariableType> results = {};
    std::vector<Instruction*>* body = nullptr;
} AST_Function;


#endif
