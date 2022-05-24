// Based on example on Toledo

#include "bytestream.h"
#include <cstdint>
#include <algorithm>

class Character {
    
    public:

        // https://webassembly.github.io/spec/core/text/lexical.html#white-space
        static bool isWhitespace(unsigned char candidate) {
            return (candidate == ' ' || candidate == '\n' || candidate == '\t' || candidate == '\r' );
        }

        // [a-zA-Z\.0-9_]
        // https://webassembly.github.io/spec/core/text/lexical.html#characters
        // "the rest of the grammar is formed exclusively from the characters supported by the 7-bit ASCII subset of Unicode."
        // TODO: extend with other characters as well: https://webassembly.github.io/spec/core/text/values.html#text-idchar
        static bool isWASMIdentifier(unsigned char candidate) {
            return 
                (candidate >= 'a' && candidate <= 'z') ||
                (candidate >= 'A' && candidate <= 'Z') ||
                Character::isNumeric(candidate) ||
                candidate == '.' ||
                candidate == '_';
        }

        // [0-9]
        static bool isNumeric(unsigned char candidate) {
            return (candidate >= '0' && candidate <= '9');
        }
};

// TODO: support more number types than just uint32_t
enum class TokenType { BRACKETS_OPEN, BRACKETS_CLOSED, KEYWORD, STRING, NUMBER, VARIABLE };

class Token {
public:
    Token(TokenType type, std::string string_value) : type( type ), string_value( string_value ) {}
    Token(TokenType type, uint32_t uint32_value) : type( type ), uint32_value( uint32_value ) {}

    ~Token(){};

    TokenType type;

    // union was messing seriously with ability to store in std::vector<Token>, complaining about copy ctor
    // if extending with more types, should probably have a union or inheritance approach!
    // union {
    //     std::string string_value;
    //     uint32_t uint32_value;
    // };

    std::string string_value;
    uint32_t uint32_value;
};
