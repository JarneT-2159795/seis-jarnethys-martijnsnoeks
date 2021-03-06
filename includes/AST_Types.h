#ifndef _AST_FUNCTION_H_
#define _AST_FUNCTION_H_

#include <vector>
#include <string>
#include <unordered_map>
#include "instruction.h"
#include "variabletype.h"

typedef struct AST_Function {
    std::string name;
    std::vector<VariableType> parameters = {};
    std::vector<VariableType> results = {};
    std::vector<Instruction*>* body = nullptr;
    std::unordered_map<std::string, std::pair<uint32_t, uint8_t>> locals = {};
    bool isImported = false;
    std::string importModule;
    std::string importField;
} AST_Function;

typedef struct AST_Memory {
    std::string name;
    int initial_value = 0;
    int max_value = 0;
    bool isImported = false;
    std::string importModule;
    std::string importField;
} AST_Memory;

typedef struct AST_Data {
    uint8_t type;
    uint32_t value;
    std::string data;
} AST_Data;

#endif
