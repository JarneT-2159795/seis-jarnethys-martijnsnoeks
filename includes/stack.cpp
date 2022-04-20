#include "stack.h"
Stack::Stack():vector{new std::vector<Variable>()}{

}
Stack::Stack(std::vector<Variable> *vectorinit):vector{vectorinit}{

}
Variable Stack::pop(){
    Variable result = vector->back();
    vector->pop_back();
    return result;
}
int Stack::size(){
    return vector->size();
}
void Stack::push(Variable variable){
    vector->push_back(variable);
    return;
}