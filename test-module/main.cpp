#include "../includes/module.h"
#include <iostream>

int main() {
    Module m{"./test.wasm"};
    auto funcs = m.getFunctions();
    for (auto func : funcs) {
        std::cout << func.getName() << std::endl;
    }
    return 0;
}
