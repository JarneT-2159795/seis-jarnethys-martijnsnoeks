#include <vector>
#include <variant>
#include <exception>
#include <array>
#include <unordered_map>
#include "bytestream.h"
#include "constants.h"
#include "stack.h"
#include "variabletype.h"
#include "Memory.h"
#include <stack>

struct FunctionException : public std::exception{
    std::string s;
    FunctionException(std::string ss, int byte) : s(ss + " Instruction: " + std::to_string((int)byte)) {};
    FunctionException(std::string ss) : s(ss) {}
    ~FunctionException() throw () {}
    const char* what() const throw() { return s.c_str(); }
};

class GlobalVariable {
public:
    GlobalVariable(Variable var, bool isConstant) : value(var), is_constant(isConstant) {}
    const bool is_constant;
    Variable getVariable() { return value; }
    void setVariable(Variable var) { if (is_constant) throw FunctionException("Cannot set constant variable"); value = var; }

private:
    Variable value;
};

enum class blockType {
    BLOCK,
    LOOP,
    IF,
};

class Function {
public:
    Function(std::vector<VariableType> paramaterList, std::vector<VariableType> resultList, Stack *moduleStack,
             std::vector<Function> *moduleFunctions, std::vector<GlobalVariable> *moduleGlobals, std::vector<Memory> *moduleMemories);
    void setName(std::string functionName);
    std::string getName();
    std::vector<VariableType> getParams() { return params; };
    std::vector<VariableType> getResults() { return results; };
    void addLocalVars(VariableType varType, int count);
    void setBody(std::vector<uint8_t> functionBody);
    void operator()(int offset);

private:
    std::string name = "noName";
    std::vector<VariableType> params;
    std::vector<VariableType> localVars;
    std::vector<VariableType> results;
    std::vector<uint8_t> body;
    std::unordered_map<int, std::array<int, 2>> ifJumps;
    std::unordered_map<int, int> blockJumps;
    std::unordered_map<int, int> loopJumps;
    std::stack<blockType> lastBlock;
    int stackOffset = 0;
    Stack *stack;
    std::vector<Function> *functions;
    std::vector<GlobalVariable> *globals;
    std::vector<Memory> *memories;
    ByteStream bs;
    void performOperation(uint8_t byte, std::vector<int> &jumpStack, std::vector<int> &ifStack);
    bool jumpsCalculated = false;
    void findJumps();
};
