#include "stack.h"
#include <iostream>

Stack::Stack() : vector{} {
}

Stack::Stack(std::vector<Variable> *vectorinit) : vector{*vectorinit} {
}

Variable Stack::pop(){
    Variable result = vector.back();
    vector.pop_back();
    return result;
}

int Stack::size(){
    return vector.size();
}

void Stack::push(Variable variable){
    vector.push_back(variable);
    return;
}

Variable Stack::at(int index){
    return vector.at(index);
}

Variable Stack::back(){
    return vector.back();
}

void Stack::printAll(){
    for (int i = 0; i < vector.size(); ++i) {
        std::visit([&](auto&& arg){
            std::cout << arg << " ";
        }, vector.at(i));
    }
    std::cout << std::endl;
}
