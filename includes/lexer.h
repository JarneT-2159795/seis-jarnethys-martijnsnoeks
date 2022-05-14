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

public:
	Lexer(std::string path);
    Lexer(std::vector<uint8_t> stream);
    ~Lexer();
    
    int lex();

    ByteStream* getByteStream(){ return this->byteStream; }
    std::vector<Token> getTokens() { return this->tokens; }

protected:
    Token parseKeyword();
    Token parseNumber();
    Token parseString();
    void parseComment();
};

#endif // __LEXER_H__