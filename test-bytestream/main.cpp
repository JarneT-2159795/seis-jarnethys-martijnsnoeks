#include "../includes/bytestream.h"
#include <iostream>

int main() {
    ByteStream bs("./number");
    std::cout << bs.readInt64() << std::endl;
    return 0;
}