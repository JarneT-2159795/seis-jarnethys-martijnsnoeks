#include <iostream>
#include <vector>
#include <string>

#include "../includes/lexer.h"
#include "../includes/parser.h"
#include "../includes/compiler.h"

using namespace std;

int main()
{
    std::cout << "TESTING " << std::endl;

	Lexer lexer = Lexer{"./test.wat"};

    int err = lexer.lex();

    if (err != 0) {
        std::cout << "Lexer error: " << err << std::endl;
    }

    std::cout << "DONE LEXING " << std::endl;

    Parser parser = Parser(&lexer);

    std::vector<Instruction*> AST = parser.parseProper();
    auto functions = parser.getFunctions();
    
    std::cout << "DONE PARSING " << std::endl;

    Compiler compiler = Compiler(functions);
    compiler.compile();
    compiler.writeFile("output.wasm");

    std::cout << "DONE COMPILING " << std::endl;
}

