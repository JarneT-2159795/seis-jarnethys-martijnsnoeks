#include "../includes/module.h"
#include <iostream>

extern "C" {
    int useModule(uint8_t *data, int size, int *input);
    int testPrint() {
        return rand();
    }
}

int main() {
    std::cout << "Call useModule(uint8_t *data, int size)" << std::endl;
    return 0;
}


int useModule(uint8_t *data, int size, int *input) {
    if (size < 1) {
        std::cout << "Error: No data received" << std::endl;
        return -1;
    }
    ByteStream bs;
    Module module{data, size};
    auto funcs = module.getFunctions();
    int choice = 0;

    while (choice > -1 && choice < funcs.size()) {
        std::string func = funcs[choice].getName();
        Stack vars;
        for (int i = 0; i < funcs[choice].getParams().size(); i++) {
            vars.push(input[i]);
        }
        module(func, vars);
        auto result = std::get<int>(module.getResults(funcs[choice].getResults().size())[0]);
        return result;
    }
    return -1;
}
