#include <vector>
#include <variant>
#include <exception>
#include "bytestream.h"
#include "instructions.h"

#define float32_t float
#define float64_t double

enum class VariableType { is_int32, is_int64, isfloat32_t, isfloat64_t };
typedef std::variant<int32_t, int64_t, float32_t, float64_t> Variable;

class Function {
public:
    Function(std::vector<VariableType> paramaterList, std::vector<VariableType> resultList, std::vector<Variable> *moduleStack, std::vector<Function> *moduleFunctions);
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

    int stackOffset = 0;
    std::vector<Variable> *stack;
    std::vector<Function> *functions;
};

struct FunctionException : public std::exception
{
   std::string s;
   FunctionException(std::string ss, int byte) : s(ss + " Instruction: " + std::to_string((int)byte)) {};
   FunctionException(std::string ss) : s(ss) {}
   ~FunctionException() throw () {}
   const char* what() const throw() { return s.c_str(); }
};
