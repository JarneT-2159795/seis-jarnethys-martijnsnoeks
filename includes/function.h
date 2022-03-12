#include <vector>
#include "variable.h"

class Function {
public:
    Function(std::vector<Variable> paramaterList, std::vector<Variable> resultList);
    void setName(std::string functionName);
    std::string getName();
    void addLocalVars(VariableType varType, int count);
    void setBody(std::vector<uint8_t> functionBody);

private:
    std::string name;
    std::vector<Variable> params;
    std::vector<Variable> localVars;
    std::vector<Variable> results;
    std::vector<uint8_t> body;
};
