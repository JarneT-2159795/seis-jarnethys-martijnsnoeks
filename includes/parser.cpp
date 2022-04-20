// Based on example on Toledo

#include <iostream>
#include <iomanip> // std::setw(), std::setfill()
#include <vector>
#include <string>

#include "lexer.h"
#include "parser.h"
#include "instruction.h"

using namespace std;

ByteStream* Parser::parseSimple()
{
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

	output->seek(0);

	cout << "Nr bytes written " << writtenByteCount << endl;

	int c = 0;
	while( c <= writtenByteCount ) { 
		unsigned char byte = output->readByte();

		cout << std::hex << std::setw(2) << std::setfill('0') << (int) byte << std::dec << endl;

		++c;
	}

	return output;
}




std::vector<Instruction*> Parser::parseProper() 
{
	std::vector<Token> tokens = this->lexer->getTokens();

	// Proper parser will map instructions to the main calculations and their operands, if applicable

	std::vector<Instruction*> output = std::vector<Instruction*>();

	int totalLenght = 0;

	// we only want to output the function body for now, to keep it simple
	// HACK: assume that only starts with the first valid "operation" we get
	// TODO: properly parse everything
	bool HACK_inCodeBlock = false;

	for ( int i = 0; i < tokens.size(); ++i ) {
		
		Token token = tokens[i];

		if ( token.type == TokenType::KEYWORD ) {
			InstructionNumber::Operation op = InstructionNumber::getOperation( token.string_value );
			
			// cout << "Parser::ParseProper : token : " << token.string_value << " maps to " << (int) op << endl;

			if ( op != InstructionNumber::Operation::NONE ) {
				HACK_inCodeBlock = true;

				if ( InstructionNumber::isCalculation(op) ) {
					Instruction *instruction = instruction = new Instruction( InstructionType::CALCULATION );
					instruction->instruction_code = (int) op;
					output.push_back( instruction );
				}
				else if ( InstructionNumber::isConst(op) ) {
					Instruction *instruction = instruction = new Instruction( InstructionType::CONST );
					instruction->instruction_code = (int) op;
					Token parameter = tokens[++i]; // parameter MUST be next behind this
					instruction->parameter = parameter.uint32_value;

					output.push_back( instruction );
				}
				else if ( InstructionNumber::hasParameter(op) ) {
					Instruction *instruction = instruction = new Instruction( InstructionType::INSTRUCTION_WITH_PARAMETER );
					instruction->instruction_code = (int) op;
					Token parameter = tokens[++i]; // parameter MUST be next behind this
					instruction->parameter = parameter.uint32_value;

					output.push_back( instruction );
				}
				else if ( InstructionNumber::hasNoParameter(op) ) {
					Instruction *instruction = instruction = new Instruction( InstructionType::INSTRUCTION_WITHOUT_PARAMETER );
					instruction->instruction_code = (int) op;
					output.push_back( instruction );
				}
				else {
					cout << "Parser::ParseProper : unsupported operation found : " << (int) op << endl;
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
						output.push_back( instruction );
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

			cout << "Parser::ParseProper : unsupported TokenType found : " << (int) token.type << " for " << token.uint32_value << " OR " << token.string_value << endl;
		}
	}

	for ( auto instruction : output ) {
		cout << "Instruction " << (int) instruction->type << " for operation " << instruction->instruction_code << " with potential parameter " << instruction->parameter << endl;
	}

	return output;
}
