#include <iostream>
#include <vector>
#include <string>

#include "../includes/lexer.h"
#include "../includes/parser.h"
#include "../includes/compiler.h"

using namespace std;

int main()
{
    cout << "TESTING " << endl;

	Lexer lexer = Lexer{"./test.wat"};

    int err = lexer.lex();

    cout << "DONE LEXING " << endl;

    Parser parser = Parser(&lexer);

    ByteStream* compiledOutput = parser.parseSimple();
    // std::vector<Instruction*> AST = parser.parseProper();
    
    // cout << "DONE PARSING " << endl;

    // Compiler compiler = Compiler(AST);
    // ByteStream* compiledOutput = compiler.compile();

    // cout << "DONE COMPILING " << endl;
}

