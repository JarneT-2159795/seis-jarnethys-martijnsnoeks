#include <vector>
#include <variant>
#include <exception>
#include <array>
#include <unordered_map>
#include "bytestream.h"
#include "constants.h"
#include "stack.h"

#define float32_t float
#define float64_t double

enum class VariableType { is_int32, is_int64, isfloat32_t, isfloat64_t };
typedef std::variant<int32_t, int64_t, float32_t, float64_t> Variable;

class Function {
public:
    Function(std::vector<VariableType> paramaterList, std::vector<VariableType> resultList, Stack *moduleStack, std::vector<Function> *moduleFunctions);
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
    int stackOffset = 0;
    Stack *stack;
    std::vector<Function> *functions;
    ByteStream bs;
    void performOperation(uint8_t byte, std::vector<int> &jumpStack, std::vector<int> &ifStack);
    bool jumpsCalculated = false;
    void findJumps();
};

struct FunctionException : public std::exception
{
   std::string s;
   FunctionException(std::string ss, int byte) : s(ss + " Instruction: " + std::to_string((int)byte)) {};
   FunctionException(std::string ss) : s(ss) {}
   ~FunctionException() throw () {}
   const char* what() const throw() { return s.c_str(); }
};
