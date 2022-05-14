// Based on example on Toledo

#include <iostream>
#include <string>
#include <algorithm>

#include "lexer.h"

// partially inspired by https://gist.github.com/arrieta/1a309138689e09375b90b3b1aa768e20

Lexer::Lexer(std::string path) // : currentByteIndex{ new uint32_t(0) }
{
    this->byteStream = new ByteStream{path};
}

Lexer::~Lexer()
{
    delete this->byteStream;    
}


int Lexer::lex()
{
    this->tokens = std::vector<Token>();

	while( !this->byteStream->atEnd() ) {

        while( Character::isWhitespace( this->byteStream->peekByte() ) ) {
            this->byteStream->seek(1);
        }

        // if the file ends with a whitespace
        if( this->byteStream->atEnd() ) {
            break;
        }

        unsigned char nextChar = this->byteStream->peekByte();

        // we could do a switch here, but I feel if-else is cleaner, fight me

        // first check for numeric, THEN alphaNumeric (or we'd always get alphaNumeric)
        if( Character::isNumeric(nextChar) ) {
            this->tokens.push_back( this->parseNumber() );
        }
        else if ( Character::isWASMIdentifier(nextChar) ){
            this->tokens.push_back( this->parseKeyword() );
        }
        else {
            switch (nextChar) {
                case '"':
                    this->tokens.push_back( this->parseString() );
                    break;
                case '(':
                    this->tokens.emplace_back(TokenType::BRACKETS_OPEN, std::string(1, this->byteStream->readByte()) ) ;
                    break;
                case ')':
                    this->tokens.push_back( Token(TokenType::BRACKETS_OPEN, std::string(1, this->byteStream->readByte())) ) ;
                    break;
                case ';':
                    this->parseComment();
                    break;
                default:
                    std::cout << "Unknown character " << this->byteStream->peekByte() << " at byte " << this->byteStream->getCurrentByteIndex() << std::endl;
                    this->tokens.push_back( Token(TokenType::STRING, std::string(1, this->byteStream->readByte())) ) ;
                    break;
            }
        }

        // TODO: support:
        // - $variable syntax
        // - extra parameters with = (e.g., seek=XYZ for memory ops)
        // - hexadecimal numbers (e.g., 0x12) https://webassembly.github.io/spec/core/text/values.html#integers
        // - floating point numbers https://webassembly.github.io/spec/core/text/values.html#floating-point

	}
/*
    std::cout << "Token Count : " << this->tokens.size() << std::endl;

    for ( auto token : this->tokens ) {
        if ( token.type == TokenType::NUMBER ) {
            std::cout << (int) token.type << " : " << token.uint32_value << std::endl;
        }
        else {
            std::cout << (int) token.type << " : " << token.string_value << std::endl;
        }
    }
    
    std::cout << std::endl;
*/
    return 0;
}

Token Lexer::parseKeyword() 
{
    std::string val = "";

    while( Character::isWASMIdentifier(this->byteStream->peekByte()) ) {
        val.append( 1, this->byteStream->readByte() );
    }

    return Token( TokenType::KEYWORD, val );
}

Token Lexer::parseNumber()
{
    std::string val = "";

    while( Character::isNumeric(this->byteStream->peekByte()) ) {
        val.append( 1, this->byteStream->readByte() );
    }

    return Token( TokenType::NUMBER, std::stoi(val) );
}

Token Lexer::parseString() 
{
    std::string val = "";
    // first coming byte is a " (detected in ::lex), so skip that
    this->byteStream->seek(1);

    while( this->byteStream->peekByte() != '"' ) {
        val.append( 1, this->byteStream->readByte() );
    }
    
    // skip the final "
    this->byteStream->seek(1);

    return Token( TokenType::STRING, val );
}

void Lexer::parseComment()
{
    // https://webassembly.github.io/spec/core/text/lexical.html#comments
    // two types of comments
    // either it's a full line: starts with ;; and ends on \n
    // OR it's between two brackets and semicolons (;  ;)
	
    // FIXME: TODO: there's still a bug in the parsing of the (; ;)
    //  -> it leaves the first ( intact in the output instead of removing it!
    // need to revise how we do the comment parsing (if encounter (, look if there's a ; behind it first?)
    
    bool fullLine = false; // assume 2nd type

    // skip the first ;
    this->byteStream->seek(1);


    if ( this->byteStream->peekByte() == ';' ) {
        // skip the second ; (case 1: full line)
        this->byteStream->seek(1);
        fullLine = true;
    }

    bool cont = true;
    do {
        unsigned char nextChar = this->byteStream->peekByte();

        if ( fullLine ) {
            // looking just for \n
            if ( nextChar == '\n' ) {
                cont = false;
            }
            else {
                this->byteStream->readByte();
            }
        }
        else {
            // looking for ;)
            if ( nextChar == ';' ) {
                // 2 options: either it's ;)
                // or it's a ; in the middle of the comment that we need to skip
                
                // skip this ;
                this->byteStream->readByte();
                nextChar = this->byteStream->peekByte();

                if ( nextChar == ')' ) {
                    cont = false;
                }
            }
            else {
                this->byteStream->readByte();
            }
        }
    }
    while( cont );
    
    // skip the ending char (either ; or \n)
    this->byteStream->seek(1);

    return;
}

