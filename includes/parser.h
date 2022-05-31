// Based on example on Toledo

#ifndef __PARSER_H__
#define __PARSER_H__


#include "lexer.h"
#include "AST_Types.h"

class Parser {
private:
    Lexer *lexer;
    std::vector<AST_Function*> functions;
    std::vector<AST_Memory*> memories;
    AST_Function* currentFunction = nullptr;

public:
	Parser(Lexer *lexer) : lexer(lexer) {}
    ~Parser() {}
    
    void parseProper();
    std::vector<AST_Function*> getFunctions() { return functions; }
    std::vector<AST_Memory*> getMemories() { return memories; }
};

#endif // __PARSER_H__