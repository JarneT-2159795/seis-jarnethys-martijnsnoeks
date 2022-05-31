---
layout: page
title: WebAssembly compiler
permalink: /wasm-compiler
---

## Introduction

For our next task we had to write a compiler for very basic WebAssembly text files. This compiler accepts WAT files and will output WASM files. We will try to support certain features like variables with names (e.g. `$var`) instead of numbers. Furthermore we will try to perform some optimization like folding constant expressions to one line of code like this:

```js

i32.const 5
i32.const 12          =====>          i32.const 17
i32.add

```

A compiler typically consists of three parts: a lexer, a parser and the compiler itself. In this blogpost we will show what each component does and how we implemented this in C++.

## Lexer

The lexer reads the WAT file and splits it in different tokens which the parser uses. The tokens make it easier to perform operations on the code like folding. For the complete code for this part you can check out our [Lexer class](https://github.com/JarneT-2159795/seis-jarnethys-martijnsnoeks/blob/main/includes/lexer.cpp) and our [Token and Character classes](https://github.com/JarneT-2159795/seis-jarnethys-martijnsnoeks/blob/main/includes/token.h). Our Token class looks like this:

```c++

enum class TokenType { BRACKETS_OPEN, BRACKETS_CLOSED, KEYWORD, STRING, NUMBER, VARIABLE };

class Token {
public:
    Token(TokenType type, std::string string_value) : type( type ), string_value( string_value ) {}
    Token(TokenType type, uint32_t uint32_value) : type( type ), uint32_value( uint32_value ) {}
    Token(TokenType type, double double_value) : type( type ), double_value( double_value ) {}

    ~Token(){};

    TokenType type;

    std::string string_value;
    uint32_t uint32_value;
    double double_value;
};

```

As you can see we split the tokens in different groups. This will make it easier to handle them later on and perform error checking on them.

- Brackets open and brackets closed
- Keywords: this can be words like \"func\" or \"param\"
- String: this is literal string enclosed by \"\" to write a function name for example
- Number: an I32 or F32 constant number
- Variable: a variable name like `var`

The lexer uses our ByteStream class from the virtual machine in the [previous blogpost](https://jarnet-2159795.github.io/seis-jarnethys-martijnsnoeks/wasm-vm). We can do this because the WebAssembly specification notes that all characters in a WAT file have to be ASCII thus these are all 8 bit characters and can be stored in a byte in C++. When we have our ByteStream we can start to actually create tokens. The complete code for lexing a file is as follows:

```c++

int Lexer::lex() {
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
                case ')':
                    this->tokens.emplace_back(TokenType::BRACKETS_OPEN, std::string(1, this->byteStream->readByte()) ) ;
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
                default:
                    std::cout << "Unknown character " << this->byteStream->peekByte() << " at byte " << this->byteStream->getCurrentByteIndex() << std::endl;
                    this->tokens.emplace_back(TokenType::STRING, std::string(1, this->byteStream->readByte()) ) ;
                    break;
            }
        }
	}
    return 0;
}

```

As you can see we use a `std::vector` to store all of our tokens. We start a loop over the entire ByteStream and continue skipping bytes as long as we find whitespaces (e.g. \\n, \\r, \\t or an actual space). Now that we have found an actual character we can start checking which type it is. We first check if it is numeric or not. When it is numeric we can call the `parseNumber()` function to actually parse the whole number, check if it is an int or a float and store it in its appropriate variable.

```c++

static bool isNumeric(unsigned char candidate) {
    return (candidate >= '0' && candidate <= '9');
}

Token Lexer::parseNumber() {
    std::string val = "";

    while( Character::isNumeric(this->byteStream->peekByte()) || this->byteStream->peekByte() == '.' ) {
        val.append( 1, this->byteStream->readByte() );
    }

    if (val.find('.') != std::string::npos) {
        return Token(TokenType::NUMBER, std::stod(val));
    }
    return Token( TokenType::NUMBER, (uint32_t)std::stoi(val) );
}

```

When the character is not numeric we check if it is a WebAssembly keyword. A keyword is made up with a combination of the following characters:

- a - z
- A - Z
- 0 - 9
- a dot (\".\") or an underscore (\"_\")

Note that keywords do not start with a numeric character, otherwise they would be parsed as numbers. When we know it is a keyword we keep reading the next characters until we find one that is not part of any of the characters listed above (usually a space) which marks the end of the keyword.

```c++

static bool isWASMIdentifier(unsigned char candidate) {
    return 
        (candidate >= 'a' && candidate <= 'z') ||
        (candidate >= 'A' && candidate <= 'Z') ||
        Character::isNumeric(candidate) ||
        candidate == '.' ||
        candidate == '_';
}

Token Lexer::parseKeyword() {
    std::string val = "";

    while( Character::isWASMIdentifier(this->byteStream->peekByte()) ) {
        val.append( 1, this->byteStream->readByte() );
    }

    return Token( TokenType::KEYWORD, val );
}

```

When the character is neither of the above we have a couple of possibilities left which we solved with a switch case. Its either a \" which means a string will follow and we can parse this string until the next \".

```c++

Token Lexer::parseString() {
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

```

It could also be a \"(\" or \")\". These get added to the list of tokens without any additional parsing because they are always just one character. When the character is a \";\" we know we have found a comment. We can just leave these out of the list of tokens because they are of no importance to the rest of the compiler. We do however have to account for the two types of comment possible in WebAssembly. A multiline comment like `(; comment here ;)` or a single line comment like `;; comment here`. The `parseComment()` function checks the byte previous to the \";\" found. When it's a \"(\" we know we have a multiline comment and otherwise it's a single line comment. In case of a multiline comment we search for a sequence of \";)\" which ends the comment and in case of a single line comment we skip to the next line by searching \"\\n\".

```c++

void Lexer::parseComment() {
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

```

Lastly it could be that the character is \"$\" which means it is a variable name. This is parsed much like a keyword. We keep adding characters to the name string until we find a whitespace. When the character is neither of the above cases we throw an error.

```c++

Token Lexer::parseVarName() {
    std::string val;
    while ( !Character::isWhitespace(byteStream->peekByte()) ) {
        val += byteStream->readByte();
    }
    return Token(TokenType::VARIABLE, val);
}

```

## Parser

Now that we have a list of tokens we can start parsing these. If we are honest this part isn't our best work, especially in comparison with the virtual machine. We were a bit time-constrained on this part and had to build as many features as we could in the limited time we had left. That said it still works and we will explain how so when you want to do this you can do it properly :). The full code for this can be found in our repository in the [Parser class](https://github.com/JarneT-2159795/seis-jarnethys-martijnsnoeks/blob/main/includes/parser.cpp) and [Instruction and InstructionNumber classes](https://github.com/JarneT-2159795/seis-jarnethys-martijnsnoeks/blob/main/includes/instruction.h). The code below shows the main part which we will break down.

```c++

std::vector<Instruction*> Parser::parseProper() {
	std::vector<Token> tokens = this->lexer->getTokens();
	std::vector<Instruction*> *output = nullptr;

	int totalLenght = 0;
	bool HACK_inCodeBlock = false;

	for ( int i = 0; i < tokens.size(); ++i ) {		
		Token token = tokens[i];
        if (token.string_value == "func") {
            if (currentFunction != nullptr) {
                output->push_back(new Instruction(InstructionType::INSTRUCTION_WITHOUT_PARAMETER, constants::BLOCK_END));
                currentFunction->body = output;
                functions.push_back(currentFunction);
                output = new std::vector<Instruction*>();
                currentFunction = new AST_Function;
            } else {
                currentFunction = new AST_Function;
                output = new std::vector<Instruction*>;
            }
            continue;
        }
        if (token.string_value == "export") {
            i++;
            if (currentFunction != nullptr) {
                currentFunction->name = tokens[i].string_value;
            }
            continue;
        }
        if (token.string_value == "param") {
            i++;
            while (tokens[i].string_value != ")") {
                switch(InstructionNumber::getType(tokens[i].string_value)) {
                    case InstructionNumber::Type::I32:
                        currentFunction->parameters.push_back(VariableType::is_int32);
                        break;
                    case InstructionNumber::Type::I64:
                        currentFunction->parameters.push_back(VariableType::is_int64);
                        break;
                    case InstructionNumber::Type::F32:
                        currentFunction->parameters.push_back(VariableType::isfloat32_t);
                        break;
                    case InstructionNumber::Type::F64:
                        currentFunction->parameters.push_back(VariableType::isfloat64_t);
                        break;
                    default:
                        std::cout << "Unknown parameter type: " << tokens[i].string_value << std::endl;
                        break;
                }
                i++;
            }
            continue;
        }
        if (token.string_value == "result") {
            i++;
            while (tokens[i].string_value != ")") {
                switch(InstructionNumber::getType(tokens[i].string_value)) {
                    case InstructionNumber::Type::I32:
                        currentFunction->results.push_back(VariableType::is_int32);
                        break;
                    case InstructionNumber::Type::I64:
                        currentFunction->results.push_back(VariableType::is_int64);
                        break;
                    case InstructionNumber::Type::F32:
                        currentFunction->results.push_back(VariableType::isfloat32_t);
                        break;
                    case InstructionNumber::Type::F64:
                        currentFunction->results.push_back(VariableType::isfloat64_t);
                        break;
                    default:
                        std::cout << "Unknown result type: " << tokens[i].string_value << std::endl;
                }
                i++;
            }
            continue;
        }

		switch ( token.type ) {
            case TokenType::KEYWORD: {
                uint8_t op = InstructionNumber::getOperation(token.string_value);

                if (op != 0) {
                    HACK_inCodeBlock = true;

                    if (InstructionNumber::isCalculation(op)) {
                        Instruction *instruction = instruction = new Instruction(InstructionType::CALCULATION);
                        instruction->instruction_code = (int) op;
                        output->push_back(instruction);
                    } else if (InstructionNumber::isConst(op)) {
                        Instruction *instruction = instruction = new Instruction(InstructionType::CONST);
                        instruction->instruction_code = (int) op;
                        Token parameter = tokens[++i]; // parameter MUST be next behind this
                        if (op == constants::I32CONST) {
                            instruction->parameter = parameter.uint32_value;
                        } else if (op == constants::F32CONST) {
                            instruction->float_parameter = parameter.double_value;
                        }

                        output->push_back(instruction);
                    } else if (InstructionNumber::hasParameter(op)) {
                        Instruction *instruction = instruction = new Instruction(
                                InstructionType::INSTRUCTION_WITH_PARAMETER);
                        instruction->instruction_code = (int) op;
                        Token parameter = tokens[++i]; // parameter MUST be next behind this
                        if (parameter.type == TokenType::VARIABLE) {
                            instruction->parameter = currentFunction->locals[parameter.string_value].first;
                        } else {
                            instruction->parameter = parameter.uint32_value;
                        }

                        output->push_back(instruction);
                    } else if (InstructionNumber::hasNoParameter(op)) {
                        Instruction *instruction = instruction = new Instruction(
                                InstructionType::INSTRUCTION_WITHOUT_PARAMETER);
                        instruction->instruction_code = (int) op;
                        output->push_back(instruction);
                    } else {
                        std::cout << "Parser::ParseProper : unsupported operation found : " << op << std::endl;
                    }

                }
                // KEYWORD is either an instruction or a type (e.g., i32.add or just i32)
                else {
                    if (HACK_inCodeBlock) {
                        InstructionNumber::Type type = InstructionNumber::getType(token.string_value);

                        if (type != InstructionNumber::Type::NONE) {
                            // Types are always without parameter
                            Instruction *instruction = instruction = new Instruction(
                                    InstructionType::INSTRUCTION_WITHOUT_PARAMETER);
                            instruction->instruction_code = (int) type;
                            output->push_back(instruction);
                        }
                    }
                }
                break;
            }
            case TokenType::VARIABLE: {
                if (!currentFunction->locals.contains(tokens[i + 1].string_value)) {
                    uint8_t type = 0;
                    if (tokens[i + 2].string_value == "i32") type = constants::INT32;
                    else if (tokens[i + 2].string_value == "i64") type = constants::INT64;
                    else if (tokens[i + 2].string_value == "f32") type = constants::FLOAT32;
                    else if (tokens[i + 2].string_value == "f64") type = constants::FLOAT64;
                    currentFunction->locals.insert(
                                            std::make_pair(tokens[i + 1].string_value,
                                            std::make_pair(currentFunction->parameters.size() + currentFunction->locals.size(), type)));
                }
                i += 2;
                break;
            }
            case TokenType::BRACKETS_OPEN:
            case TokenType::BRACKETS_CLOSED:
                break;
            default: {
                if (!HACK_inCodeBlock) {
                    continue;
                } else {
                    std::cout << "Parser::ParseProper : unsupported TokenType found : " << (int) token.type << " for " << token.uint32_value << " OR " << token.string_value << std::endl;
                }
            }
		}
	}
    if (currentFunction != nullptr) {
        output->push_back(new Instruction(InstructionType::INSTRUCTION_WITHOUT_PARAMETER, constants::BLOCK_END));
        currentFunction->body = output;
        functions.push_back(currentFunction);
    }
	return *output;
}

```

First we create a vector of instructions which will later be passed tot the actual compiler to convert to a WASM file. Next we will start looping over our vector of tokens. First we perform a few basic checks to see if it is the start of a function.

### The \"func\" keyword

When we find the start of a function we first check if the pointer to the current function is NULL. If so this is the first function, else we add 0xB to the end of the previous function and add it to the list of functions.

```c++

if (token.string_value == "func") {
    if (currentFunction != nullptr) {
        output->push_back(new Instruction(InstructionType::INSTRUCTION_WITHOUT_PARAMETER, constants::BLOCK_END));
        currentFunction->body = output;
        functions.push_back(currentFunction);
        output = new std::vector<Instruction*>();
        currentFunction = new AST_Function;
    } else {
        currentFunction = new AST_Function;
        output = new std::vector<Instruction*>;
    }
    continue;
}

```

### The \"export\" keyword

The export function is followed by a string which indicates the function name if we are in a function. We can later expand this by adding support for the export of memory.

```c++

if (token.string_value == "export") {
    i++;
    if (currentFunction != nullptr) {
        currentFunction->name = tokens[i].string_value;
    }
    continue;
}

```

### The \"param\" and \"result\" keyword

These keywords are followed by i32 or f32. We already added support for i64 and f64 here but didn't implement these later in the compiler but that would be pretty easy as it's basically copy-pasting. Unfortunately we didn't have time to implement the use of named variables like `$var` here so these have to be called by their index.

```c++

if (token.string_value == "param") {
    i++;
    while (tokens[i].string_value != ")") {
        switch(InstructionNumber::getType(tokens[i].string_value)) {
            case InstructionNumber::Type::I32:
                currentFunction->parameters.push_back(VariableType::is_int32);
                break;
            case InstructionNumber::Type::I64:
                currentFunction->parameters.push_back(VariableType::is_int64);
                break;
            case InstructionNumber::Type::F32:
                currentFunction->parameters.push_back(VariableType::isfloat32_t);
                break;
            case InstructionNumber::Type::F64:
                currentFunction->parameters.push_back(VariableType::isfloat64_t);
                break;
            default:
                std::cout << "Unknown parameter type: " << tokens[i].string_value << std::endl;
                break;
        }
        i++;
    }
    continue;
}
if (token.string_value == "result") {
    i++;
    while (tokens[i].string_value != ")") {
        switch(InstructionNumber::getType(tokens[i].string_value)) {
            case InstructionNumber::Type::I32:
                currentFunction->results.push_back(VariableType::is_int32);
                break;
            case InstructionNumber::Type::I64:
                currentFunction->results.push_back(VariableType::is_int64);
                break;
            case InstructionNumber::Type::F32:
                currentFunction->results.push_back(VariableType::isfloat32_t);
                break;
            case InstructionNumber::Type::F64:
                currentFunction->results.push_back(VariableType::isfloat64_t);
                break;
            default:
                std::cout << "Unknown result type: " << tokens[i].string_value << std::endl;
        }
        i++;
    }
    continue;
}

```

### Other keywords

When it's neither of the above we check the other type of keywords in a function. The `getOperation()` function in our InstructionNumber class simply has a list which returns the OP-code for the given string. For this OP-code we perform further checks as to which type of OP-code it is. We have OP-codes which take a parameter and some which take none. First we check if it's a calculation. These take no parameters as the variables on which the calculation takes place are loaded on the stack beforehand. Another option is that we have a const. This is a keyword followed by the actual value. Here we check whether it is an integer or a float and store the OP-code with it's number in the instruction. Other then these two special cases we have the two general cases of a function with or without a parameter. For a function without a parameter we just store the OP-code. When we have an OP-code with a parameter we check if this parameter is just a number or a name for a variable of the type `$var`. In the latter case we already convert this name to the index it should be.

```c++

if (InstructionNumber::isCalculation(op)) {
    Instruction *instruction = instruction = new Instruction(InstructionType::CALCULATION);
    instruction->instruction_code = (int) op;
    output->push_back(instruction);
} else if (InstructionNumber::isConst(op)) {
    Instruction *instruction = instruction = new Instruction(InstructionType::CONST);
    instruction->instruction_code = (int) op;
    Token parameter = tokens[++i]; // parameter MUST be next behind this
    if (op == constants::I32CONST) {
        instruction->parameter = parameter.uint32_value;
    } else if (op == constants::F32CONST) {
        instruction->float_parameter = parameter.double_value;
    }

    output->push_back(instruction);
} else if (InstructionNumber::hasParameter(op)) {
    Instruction *instruction = instruction = new Instruction(
            InstructionType::INSTRUCTION_WITH_PARAMETER);
    instruction->instruction_code = (int) op;
    Token parameter = tokens[++i]; // parameter MUST be next behind this
    if (parameter.type == TokenType::VARIABLE) {
        instruction->parameter = currentFunction->locals[parameter.string_value].first;
    } else {
        instruction->parameter = parameter.uint32_value;
    }

    output->push_back(instruction);
} else if (InstructionNumber::hasNoParameter(op)) {
    Instruction *instruction = instruction = new Instruction(
            InstructionType::INSTRUCTION_WITHOUT_PARAMETER);
    instruction->instruction_code = (int) op;
    output->push_back(instruction);
} else {
    std::cout << "Parser::ParseProper : unsupported operation found : " << op << std::endl;
}

```

### Variables

To parse the local variables of a function we keep a hashmap of all variables. The key of this map is the name of the variable as a string. The value is a pair of an int32 and an uint8. The int32 is the index of the variable and the uint8 indicates the type of the variable. These are stored in the function te be used later by the actual compiler.

```c++

if (!currentFunction->locals.contains(tokens[i + 1].string_value)) {
    uint8_t type = 0;
    if (tokens[i + 2].string_value == "i32") type = constants::INT32;
    else if (tokens[i + 2].string_value == "i64") type = constants::INT64;
    else if (tokens[i + 2].string_value == "f32") type = constants::FLOAT32;
    else if (tokens[i + 2].string_value == "f64") type = constants::FLOAT64;
    currentFunction->locals.insert(
                            std::make_pair(tokens[i + 1].string_value,
                            std::make_pair(currentFunction->parameters.size() + currentFunction->locals.size(), type)));
}
i += 2;
break;

```

## Compiler

The compiler will read all the information from the parser and write the actual bytes with the right syntax. We will explain each section more in depth here. The full code can be found in our [Compiler class](https://github.com/JarneT-2159795/seis-jarnethys-martijnsnoeks/blob/main/includes/compiler.cpp)

### Type section

The first byte is 0x01 as this is the identifier for this section. After this we write a zero and remember the location of this zero as this will later become the length of this section. We don't know in advance how long a section is so we store the initial length and will check the difference after we have written the whole section. We will do this for all future section too as this is standard for each one.

To start we first read the type of each function. We store each unique type in a vector for later use. A type consists of a list for all the parameters of a function and all the results of a function. Next we write the number of function types we have. A function type always starts with the byte 0x60. After this we write the amount of parameters followed by the code for each byte. Remember from the virtual machine the four types i32, i64, f32 and f64 have the code 0x7F, 0x7E, 0x7D and 0x7C respectfully. The same is done for the results of the type.

```c++

void Compiler::writeTypeSection() {
    // find function types
    for (auto function : functions) {
        bool exists = false;
        for (auto type : functionTypes) {
            if (type[0] == function->parameters && type[1] == function->results) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            auto arr = std::array<std::vector<VariableType>, 2>();
            arr[0] = function->parameters;
            arr[1] = function->results;
            functionTypes.push_back(arr);
        }
    }

    fullOutput->writeByte(functionTypes.size());
    for (auto type : functionTypes) {
        fullOutput->writeByte(0x60);
        fullOutput->writeByte(type[0].size());
        for (auto par : type[0]) {
            switch (par) {
                case VariableType::is_int32:
                    fullOutput->writeByte(constants::INT32);
                    break;
                case VariableType::is_int64:
                    fullOutput->writeByte(constants::INT64);
                    break;
                case VariableType::isfloat32_t:
                    fullOutput->writeByte(constants::FLOAT32);
                    break;
                case VariableType::isfloat64_t:
                    fullOutput->writeByte(constants::FLOAT64);
                    break;
            }
        }
        fullOutput->writeByte(type[1].size());
        for (auto par : type[1]) {
            switch (par) {
                case VariableType::is_int32:
                    fullOutput->writeByte(constants::INT32);
                    break;
                case VariableType::is_int64:
                    fullOutput->writeByte(constants::INT64);
                    break;
                case VariableType::isfloat32_t:
                    fullOutput->writeByte(constants::FLOAT32);
                    break;
                case VariableType::isfloat64_t:
                    fullOutput->writeByte(constants::FLOAT64);
                    break;
            }
        }
    }
}

```

### Function section

The function section is relatively simple. It has section code 0x03 and the length can actually be calculated in advance. It is just the amount of function types + 1. The +1 is because we first write a byte with the amount of function types. After this we loop over our list of functions and check with which type it corresponds. The index of this type is written.

```c++

fullOutput->writeByte(0x03);
fullOutput->writeByte(1 + functions.size());
fullOutput->writeByte(functions.size());
for (auto func : functions) {
    for (int i = 0; i < functionTypes.size(); i++) {
        auto type = functionTypes[i];
        if (type[0] == func->parameters && type[1] == func->results) {
            fullOutput->writeByte(i);
            break;
        }
    }
}

```

### Export section

The export section has section code 0x07 and uses the fixup method for the length of the section just like we explained earlier. For now this handles just functions but as we expand our compiler we can export other things like memory. We first write the number of functions we want to export. For each function we write the amount of characters in the name and we write each character of the name to the ByteStream. Finally we write the index of the function to which the name belongs.

```c++

void Compiler::writeExportSection() {
    fullOutput->writeByte(functions.size());
    for (int i = 0; i < functions.size(); i++) {
        fullOutput->writeByte(functions[i]->name.size());
        for (auto c : functions[i]->name) {
            fullOutput->writeByte(c);
        }
        fullOutput->writeByte(0); // TODO add different exports
        fullOutput->writeByte(i);
    }
}

```

### Code section

Here we will write the actual bodies of the functions. The section code is 0x0A and the length of the section is followed by the number of functions. The following is done for each function. We first call our `foldConstants()` function. This will simplify certain blocks of code to a single command if it is constant. Next we write a zero byte again as we need to write the length of a function body. For this we use the same fixup method as before.

Next we write all local variables. For this we first write the total number of locals followed by two bytes for each local. The first is always 1 otherwise we had to implement some more complicated logic to count all different variables. This is followed by the code for the type of variable.

```c++

fullOutput->writeByte(function->locals.size());
for (const auto& local : function->locals) {
    fullOutput->writeByte(1);
    fullOutput->writeByte(local.second.second);
}

```

Now we start writing the actual code that makes our function. We start looping all instructions in the body and write the byte that is stored is the OP-code. Next we check if the operation has a parameter. If so we also write the parameter. If the operation is a constant we have to write the value of the constant too. Remember the joy of interpreting the integers in our virtual machine because of the LEB128 encoding? As a quick reminder: each byte only contains seven actual data bits and one bit that indicates if more bytes follow. Now we have to encode these ourselves. We have a variable `bool more` which indicates whether more bytes will follow. In the while loop we handle seven bits at a time. This is he reason we perform a bitwise and with `0b0111'1111`. This will give us the value of the seven rightmost bits. After this we shift the bits seven positions to the right for when we need the next seven bits. We check if the remaining value is either 0 and the sign bit is not set or the remaining value is -1 and the sign bit is set. If either is true we don't need to write any more bytes and set more to false, otherwise we set the leftmost bit high to indicate that more bytes will follow. Luckily floating point values are much easier to write as these don't have any special encoding. We use the `reinterpret_cast<new_type&>(var)` function to take the bits of the floating point number and store them in a 32 bit integer. This lets us use the bitwise operators again. We just perform the same operation four times for each byte of the 32 bit number. We perform a bitwise and with `0b1111'1111` to get the rightmost eight bits and write this number to the output. Next we shift eight bits to the right to prepare the nxt byte.

```c++

for ( auto instruction : body ) {
    fullOutput->writeByte( instruction->instruction_code );

    if ( instruction->type == InstructionType::INSTRUCTION_WITH_PARAMETER ) {
        fullOutput->writeUInt32( instruction->parameter );
    } else if ( instruction->type == InstructionType::CONST ) {
        switch (instruction->instruction_code) {
            case constants::I32CONST:
                {
                    int32_t num = instruction->parameter;
                    if (num == 0) {
                        fullOutput->writeByte(0);
                    } else {
                        bool more = true;
                        while (more) {
                            uint8_t byte = num & 0b0111'1111;
                            num >>= 7;
                            if ((num == 0 && (byte & 0b0100'0000) == 0) || (num == -1 && (byte & 0b0100'0000) != 0)) {
                                more = false;
                            } else {
                                byte |= 0b1000'0000;
                            }
                            fullOutput->writeByte(byte);
                        }
                    }
                    break;
                }
            case constants::F32CONST:
                {
                    uint32_t num = reinterpret_cast<uint32_t&>(instruction->float_parameter);
                    for (int i = 0; i < 4; ++i) {
                        fullOutput->writeByte(num & 0xFF);
                        num >>= 8;
                    }
                }
        }
    }
}

```
