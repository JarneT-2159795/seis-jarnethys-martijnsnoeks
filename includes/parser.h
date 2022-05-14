// Based on example on Toledo

#ifndef __PARSER_H__
#define __PARSER_H__


#include "lexer.h"
#include "AST_Function.h"

class Parser {
private:
    Lexer *lexer;
    std::vector<AST_Function*> functions;
    AST_Function* currentFunction = nullptr;

public:
	Parser(Lexer *lexer) : lexer(lexer) {}
    ~Parser() {}
    
    //ByteStream* parseSimple();
    std::vector<Instruction*> parseProper();
    std::vector<AST_Function*> getFunctions() { return functions; }
};

#endif // __PARSER_H__