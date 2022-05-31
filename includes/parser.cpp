// Based on example on Toledo

#include <iostream>
#include <iomanip> // std::setw(), std::setfill()
#include <vector>
#include <string>

#include "lexer.h"
#include "parser.h"
#include "instruction.h"

using namespace std;

void Parser::parseProper() {
	std::vector<Token> tokens = this->lexer->getTokens();

	// Proper parser will map instructions to the main calculations and their operands, if applicable

	std::vector<Instruction*> *output = nullptr;

	int totalLenght = 0;

	// we only want to output the function body for now, to keep it simple
	// HACK: assume that only starts with the first valid "operation" we get
	// TODO: properly parse everything
	bool HACK_inCodeBlock = false;

	for ( int i = 0; i < tokens.size(); ++i ) {
        //std::cout << "Token " << i << ": " << tokens[i].string_value << std::endl;

		Token token = tokens[i];

        if (token.string_value == "module") {
            continue;
        }

        if (token.string_value == "memory") {
            AST_Memory* memory;
            // only one memory block allowed in current WA spec
            if (memories.empty()) {
                memory = new AST_Memory;
            } else {
                memory = memories[0];
            }
            if (tokens[i + 2].string_value == "export") {
                memory->name = tokens[i + 3].string_value;
                i += 5;
            } else {
                ++i;
            }
            memory->initial_value = tokens[i].uint32_value;
            ++i;
            if (tokens[i].type == TokenType::NUMBER) {
                memory->max_value = tokens[i].uint32_value;
                i+=2;
            }
            if (!memory->isImported) {
                memories.push_back(memory);
            }
            continue;
        }

        if (token.string_value == "import") {
            std::string module = tokens[i + 1].string_value;
            std::string field = tokens[i + 2].string_value;
            i += 3;
            if (tokens[i + 1].string_value == "memory") {
                auto memory = new AST_Memory;
                memory->isImported = true;
                memory->importModule = module;
                memory->importField = field;
                memories.push_back(memory);
                continue;
            } else if(tokens[i + 1].string_value == "func") {
                auto func = new AST_Function;
                func->isImported = true;
                func->importModule = module;
                func->importField = field;
                currentFunction = func;
                i++;
                continue;
            }
        }

        if (token.string_value == "func") {
            if (currentFunction != nullptr) {
                if (!currentFunction->isImported) {
                    output->push_back(new Instruction(InstructionType::INSTRUCTION_WITHOUT_PARAMETER, constants::BLOCK_END));
                    currentFunction->body = output;
                    functions.push_back(currentFunction);
                    output = new std::vector<Instruction*>();
                    currentFunction = new AST_Function;
                } else {
                    functions.push_back(currentFunction);
                    output = new std::vector<Instruction*>();
                    currentFunction = new AST_Function;
                }
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
                        if (op == constants::I32STORE || op == constants::I32LOAD) {
                            i += 3;
                            instruction->parameter = tokens[i].uint32_value;
                        } else {
                            Token parameter = tokens[++i]; // parameter MUST be next behind this
                            if (parameter.type == TokenType::VARIABLE) {
                                instruction->parameter = currentFunction->locals[parameter.string_value].first;
                            } else {
                                instruction->parameter = parameter.uint32_value;
                            }
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
			// FIXME: TODO: properly support these, some of these should become END instructions!!!
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
        if (!currentFunction->isImported) {
            output->push_back(new Instruction(InstructionType::INSTRUCTION_WITHOUT_PARAMETER, constants::BLOCK_END));
            currentFunction->body = output;
            functions.push_back(currentFunction);
        } else {
            functions.push_back(currentFunction);
        }
    }
/*
	for ( auto instruction : output ) {
		std::cout << "Instruction " << (int) instruction->type << " for operation " << instruction->instruction_code << " with potential parameter " << instruction->parameter << std::endl;
	}
*/
}
