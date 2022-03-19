#include "../includes/module.h"
#include <iostream>

int chooseFunction(std::vector<Function> &funcs);
Variable getVariable(VariableType vt);

int main() {
    ByteStream bs;
    Module module{"./test.wasm"};
    auto funcs = module.getFunctions();
    int choice = chooseFunction(funcs);

    while (choice > -1 && choice < funcs.size()) {
        std::string func = funcs[choice].getName();
        std::vector<Variable> vars;
        for (auto param : funcs[choice].getParams()) {
            vars.push_back(getVariable(param));
        }
        module(func, vars);
        std::cout << "Results: ";
        module.printVariables(funcs[choice].getResults().size());
        std::cout << "\n\n" << std::endl;
        choice = chooseFunction(funcs);
    }
    
    return 0;
}

int chooseFunction(std::vector<Function> &funcs) {
    std::cout << "Available functions: " << std::endl;
    for (int i = 0; i < funcs.size(); ++i) {
        std::cout << i + 1 << ") " << funcs[i].getName() << "\t parameters: ";
        for (auto param : funcs[i].getParams()) {
            switch (param) {
                case VariableType::is_int32:
                    std::cout << "i32 ";
                    break;
                case VariableType::is_int64:
                    std::cout << "i64 ";
                    break;
                case VariableType::isfloat32_t:
                    std::cout << "f32 ";
                    break;
                case VariableType::isfloat64_t:
                    std::cout << "f64 ";
                    break;
            }
        }
        std::cout << "\tresults: ";
        for (auto result : funcs[i].getResults()) {
            switch (result) {
                case VariableType::is_int32:
                    std::cout << "i32 ";
                    break;
                case VariableType::is_int64:
                    std::cout << "i64 ";
                    break;
                case VariableType::isfloat32_t:
                    std::cout << "f32 ";
                    break;
                case VariableType::isfloat64_t:
                    std::cout << "f64 ";
                    break;
            }
        }

        std::cout << std::endl;
    }
    std::cout << "Choose function (1 - " << funcs.size() << "): ";
    
    int choice;
    std::cin >> choice;
    std::cout << std::endl;
    return choice - 1;
}

Variable getVariable(VariableType vt) {
    switch (vt) {
        case VariableType::is_int32:
            {
                int32_t val;
                std::cout << "Enter parameter i32: ";
                std::cin >> val;
                return val;
            }
        case VariableType::is_int64:
            {
                int64_t val;
                std::cout << "Enter parameter i64: ";
                std::cin >> val;
                return val;
            }
        case VariableType::isfloat32_t:
            {
                float32_t val;
                std::cout << "Enter parameter f32: ";
                std::cin >> val;
                return val;
            }
        case VariableType::isfloat64_t:
            {
                float64_t val;
                std::cout << "Enter parameter f64: ";
                std::cin >> val;
                return val;
            }
        default:
            return 0;
            break;
    }
}
