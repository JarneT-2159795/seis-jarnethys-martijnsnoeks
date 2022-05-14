// Based on example on Toledo

#include <iostream>
#include <iomanip> // std::setw(), std::setfill()
#include <vector>
#include <string>

#include "lexer.h"
#include "parser.h"
#include "instruction.h"

using namespace std;
/*
ByteStream* Parser::parseSimple() {
	std::vector<Token> tokens = this->lexer->getTokens();

	// Simple parser will just go through the tokens and output them as they appear!
	// TODO: support other things than just code!

	// TODO: now we just make a big bytestream of size 2000 bytes filled with \0
	// this should be better (e.g., grow dynamically as we compile more in increments!)
	ByteStream* output = new ByteStream( std::vector<unsigned char>(2000, '\0') );
	int totalLenght = 0;

	// we only want to output the function body for now, to keep it simple
	// HACK: assume that only starts with the first valid "operation" we get
	// TODO: properly parse everything
	bool HACK_inCodeBlock = false;
	for ( auto token : tokens ) {
		if ( token.type == TokenType::KEYWORD ) {
			InstructionNumber::Operation op = InstructionNumber::getOperation( token.string_value );

			if ( op != InstructionNumber::Operation::NONE ) {
				HACK_inCodeBlock = true;

				output->writeUInt32( (uint32_t) op );
			}
			// KEYWORD is either an instruction or a type (e.g., i32.add or just i32)
			else {
				if ( HACK_inCodeBlock ) {
					InstructionNumber::Type type = InstructionNumber::getType( token.string_value );

					if ( type != InstructionNumber::Type::NONE ) {
						output->writeUInt32( (uint32_t) op );
					}
				}
			}
		}

		if ( !HACK_inCodeBlock ) {
			continue;
		}

		if ( token.type == TokenType::NUMBER ) {
			// TODO: support more than just uint32_t!
			output->writeUInt32( token.uint32_value );
		}

		// TODO: support strings and proper blocks!
		// especially the "end" of the function should be derived from its ending BRACKET_CLOSED, not an actual "end" statement
	}

	int writtenByteCount = output->getCurrentByteIndex() - 1; // -1 because byteIndex is ready to write a new byte now!

	output->setByteIndex(0);

	std::cout << "Nr bytes written " << writtenByteCount << std::endl;

	int c = 0;
	while( c <= writtenByteCount ) { 
		unsigned char byte = output->readByte();

		std::cout << std::hex << std::setw(2) << std::setfill('0') << (int) byte << std::dec << std::endl;

		++c;
	}

	return output;
}


*/

std::vector<Instruction*> Parser::parseProper() {
	std::vector<Token> tokens = this->lexer->getTokens();

	// Proper parser will map instructions to the main calculations and their operands, if applicable

	std::vector<Instruction*> *output = nullptr;

	int totalLenght = 0;

	// we only want to output the function body for now, to keep it simple
	// HACK: assume that only starts with the first valid "operation" we get
	// TODO: properly parse everything
	bool HACK_inCodeBlock = false;

	for ( int i = 0; i < tokens.size(); ++i ) {
        std::cout << "Token " << i << ": " << tokens[i].string_value << std::endl;
		
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
                }
                i++;
            }
            continue;
        }

		if ( token.type == TokenType::KEYWORD ) {
			uint8_t op = InstructionNumber::getOperation( token.string_value );
			
			// std::cout << "Parser::ParseProper : token : " << token.string_value << " maps to " << (int) op << std::endl;

			if ( op != 0 ) {
				HACK_inCodeBlock = true;

				if ( InstructionNumber::isCalculation(op) ) {
					Instruction *instruction = instruction = new Instruction( InstructionType::CALCULATION );
					instruction->instruction_code = (int) op;
					output->push_back( instruction );
				}
				else if ( InstructionNumber::isConst(op) ) {
					Instruction *instruction = instruction = new Instruction( InstructionType::CONST );
					instruction->instruction_code = (int) op;
					Token parameter = tokens[++i]; // parameter MUST be next behind this
					instruction->parameter = parameter.uint32_value;

					output->push_back( instruction );
				}
				else if ( InstructionNumber::hasParameter(op) ) {
					Instruction *instruction = instruction = new Instruction( InstructionType::INSTRUCTION_WITH_PARAMETER );
					instruction->instruction_code = (int) op;
					Token parameter = tokens[++i]; // parameter MUST be next behind this
					instruction->parameter = parameter.uint32_value;

					output->push_back( instruction );
				}
				else if ( InstructionNumber::hasNoParameter(op) ) {
					Instruction *instruction = instruction = new Instruction( InstructionType::INSTRUCTION_WITHOUT_PARAMETER );
					instruction->instruction_code = (int) op;
					output->push_back( instruction );
				}
				else {
					std::cout << "Parser::ParseProper : unsupported operation found : " << (int) op << std::endl;
				}

			}
			// KEYWORD is either an instruction or a type (e.g., i32.add or just i32)
			else {
				if ( HACK_inCodeBlock ) {
					InstructionNumber::Type type = InstructionNumber::getType( token.string_value );

					if ( type != InstructionNumber::Type::NONE ) {
						// Types are always without parameter
						Instruction *instruction = instruction = new Instruction( InstructionType::INSTRUCTION_WITHOUT_PARAMETER );
						instruction->instruction_code = (int) type;
						output->push_back( instruction );
					}
				}
			}
		}
		else if ( token.type == TokenType::BRACKETS_OPEN || token.type == TokenType::BRACKETS_CLOSED ) {
			// FIXME: TODO: properly support these, some of these should become END instructions!!!
		}
		else {
			if ( !HACK_inCodeBlock ) {
				continue;
			}

			std::cout << "Parser::ParseProper : unsupported TokenType found : " << (int) token.type << " for " << token.uint32_value << " OR " << token.string_value << std::endl;
		}
	}
    if (currentFunction != nullptr) {
        output->push_back(new Instruction(InstructionType::INSTRUCTION_WITHOUT_PARAMETER, constants::BLOCK_END));
        currentFunction->body = output;
        functions.push_back(currentFunction);
    }
/*
	for ( auto instruction : output ) {
		std::cout << "Instruction " << (int) instruction->type << " for operation " << instruction->instruction_code << " with potential parameter " << instruction->parameter << std::endl;
	}
*/
	return *output;
}
