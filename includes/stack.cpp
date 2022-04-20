#include "stack.h"
Stack::Stack():vector{new std::vector<Variable>()}{

}
Variable Stack::pop(){
    Variable result = vector->back();
    vector->pop_back();
    return result;
}
void Stack::push(Variable variable){
    vector->push_back(variable);
    return;
}