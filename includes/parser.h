// Based on example on Toledo

#ifndef __PARSER_H__
#define __PARSER_H__


#include "lexer.h"
#include "instruction.h"

class Parser
{
private:
    Lexer *lexer;

public:
	Parser(Lexer *lexer) : lexer(lexer) {}
    ~Parser() {}
    
    ByteStream* parseSimple();
    std::vector<Instruction*> parseProper();
};

#endif // __PARSER_H__