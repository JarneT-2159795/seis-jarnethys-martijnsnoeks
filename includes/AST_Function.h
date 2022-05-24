#ifndef _AST_FUNCTION_H_
#define _AST_FUNCTION_H_

#include <vector>
#include <string>
#include <unordered_map>
#include "instruction.h"

enum class VariableType { is_int32, is_int64, isfloat32_t, isfloat64_t };

typedef struct AST_Function {
    std::string name;
    std::vector<VariableType> parameters = {};
    std::vector<VariableType> results = {};
    std::vector<Instruction*>* body = nullptr;
    std::unordered_map<std::string, std::pair<uint32_t, uint8_t>> locals = {};
} AST_Function;


#endif
