#include <vector>
#include <variant>
#define float32_t float
#define float64_t double
typedef std::variant<int, long long, float32_t, float64_t> Variable;

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