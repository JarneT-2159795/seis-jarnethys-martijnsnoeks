#include <vector>
#include <stack>
#include "variable.h"
#include "bytestream.h"

class Function {
public:
    Function(std::vector<Variable> paramaterList, std::vector<Variable> resultList, std::stack<Variable> *s);
    void setName(std::string functionName);
    std::string getName();
    std::vector<Variable> getParams() { return params; };
    void addLocalVars(VariableType varType, int count);
    void setBody(std::vector<uint8_t> functionBody);
    void operator()(std::vector<Variable> locals);

private:
    std::string name;
    std::vector<Variable> params;
    std::vector<Variable> localVars;
    std::vector<Variable> results;
    std::vector<uint8_t> body;
    std::stack<Variable> *st;
};

struct FunctionException : public std::exception
{
   std::string s;
   FunctionException(std::string ss, int byte) : s(ss + " Instruction: " + std::to_string((int)byte)) {}
   FunctionException(std::string ss) : s(ss) {}
   ~FunctionException() throw () {}
   const char* what() const throw() { return s.c_str(); }
};
