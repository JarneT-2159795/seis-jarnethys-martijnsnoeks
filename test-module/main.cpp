#include "../includes/module.h"
#include <iostream>

int main() {
    ByteStream bs;
    Module module{"./test.wasm"};
    module("add", 50);
    std::cout << module.getStack().top().get() << std::endl;
    return 0;
}
