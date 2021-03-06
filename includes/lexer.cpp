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

Lexer::Lexer(std::vector<uint8_t> stream) {
    this->byteStream = new ByteStream();
    byteStream->readCharVector(stream);
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
                    this->tokens.emplace_back(TokenType::BRACKETS_CLOSED, std::string(1, this->byteStream->readByte()) ) ;
                    break;
                case ';':
                    this->parseComment();
                    break;
                case '$':
                    if (tokens.back().string_value == "local") {
                        tokens.back().type = TokenType::VARIABLE;
                    }
                    tokens.push_back(this->parseVarName());
                    break;
                    case '=':
                        this->tokens.emplace_back(TokenType::KEYWORD, std::string(1, this->byteStream->readByte()));
                        break;
                case '-':
                    byteStream->seek(1);
                    if (Character::isNumeric(byteStream->peekByte())) {
                        byteStream->seek(-1);
                        this->tokens.push_back( this->parseNumber() );
                    }
                    break;
                default:
                    std::cout << "Unknown character " << this->byteStream->peekByte() << " at byte " << this->byteStream->getCurrentByteIndex() << std::endl;
                    this->tokens.emplace_back(TokenType::STRING, std::string(1, this->byteStream->readByte()) ) ;
                    break;
            }
        }
	}
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

    while( Character::isNumeric(this->byteStream->peekByte()) || this->byteStream->peekByte() == '.' || this->byteStream->peekByte() == 'x' ||
            this->byteStream->peekByte() == '-' ) {
        val.append( 1, this->byteStream->readByte() );
    }

    if (val.find('.') != std::string::npos) {
        return Token(TokenType::NUMBER, std::stod(val));
    } else if (val.find('x') != std::string::npos) {
        return Token(TokenType::NUMBER, (uint32_t)std::stoul(val, nullptr, 16));
    }
    return Token( TokenType::NUMBER, (uint32_t)std::stoi(val) );
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

    byteStream->seek(-1);
    if (byteStream->readByte() == '(') {
        // bracket comment
        bool foundEnd = false;
        while (!foundEnd) {
            if (byteStream->readByte() == ';' && byteStream->peekByte() == ')') {
                foundEnd = true;
            }
        }
    } else {
        // full line comment
        while (byteStream->peekByte() != '\n') {
            byteStream->seek(1);
        }
    }
}

Token Lexer::parseVarName() {
    std::string val;
    while ( !Character::isWhitespace(byteStream->peekByte()) ) {
        val += byteStream->readByte();
    }
    return Token(TokenType::VARIABLE, val);
}

