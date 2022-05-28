#include <vector>
#include <variant>
#include "Variable.h"

class Stack {

private:
    std::vector<Variable> vector;

public:
    Stack();
    Stack(std::vector<Variable> *vector);

    template<typename T>
    T pop() {
        T value = std::get<T>(vector.back());
        vector.pop_back();
        return value;
    }
    Variable pop();
    int size();
    void push(Variable var);
    Variable& at(int index);
    Variable back();
    void printAll();
    void removeRange(int start, int end);
};