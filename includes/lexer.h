// Based on example on Toledo

#ifndef __LEXER_H__
#define __LEXER_H__

#include "bytestream.h"
#include "token.h"

class Lexer
{
private:
	ByteStream *byteStream;
    std::vector<Token> tokens;
    Token parseKeyword();
    Token parseNumber();
    Token parseString();
    Token parseVarName();
    void parseComment();

public:
	Lexer(std::string path);
    ~Lexer();
    
    int lex();

    ByteStream* getByteStream(){ return this->byteStream; }
    std::vector<Token> getTokens() { return this->tokens; }

};

#endif // __LEXER_H__