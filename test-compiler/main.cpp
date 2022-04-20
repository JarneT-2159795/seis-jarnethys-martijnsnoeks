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

    std::cout << "DONE LEXING " << std::endl;

    Parser parser = Parser(&lexer);

    ByteStream* compiledOutput = parser.parseSimple();
    std::vector<Instruction*> AST = parser.parseProper();
    
    std::cout << "DONE PARSING " << std::endl;

    Compiler compiler = Compiler(AST);
    compiledOutput = compiler.compile();

    std::cout << "DONE COMPILING " << std::endl;
}

