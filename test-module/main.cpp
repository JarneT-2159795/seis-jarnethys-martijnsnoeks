#include "../includes/module.h"
#include <iostream>

int main() {
    ByteStream bs;
    Module module{"./test.wasm"};
    srand(time(NULL));
    for (int i = 0; i < 10; ++i) {
        int var1{ rand() % 1000 }, var2 { rand() % 1000 };
        module("add", var1);
        std::cout << var1 << "\t+ 10 +\t" << var1 << " =\t" << module.getStack().top().get() << "\t" << var1 + 10 + var1 <<  std::endl;
    }
    return 0;
}
