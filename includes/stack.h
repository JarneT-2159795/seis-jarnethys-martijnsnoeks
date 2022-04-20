#include <vector>
#include <variant>
#define float32_t float
#define float64_t double
typedef std::variant<int32_t, int64_t, float32_t, float64_t> Variable;

class Stack{

private:
    std::vector<Variable> *vector;
public:
    Stack();
    Stack(std::vector<Variable> *vector);
    Variable pop();
    int size();
    void push(Variable var);
    Variable at(int index);
    Variable back();
};