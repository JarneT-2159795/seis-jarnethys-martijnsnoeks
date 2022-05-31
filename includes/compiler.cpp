// Based on example on Toledo

#include <iostream>
#include <iomanip> // std::setw(), std::setfill()
#include <vector>
#include <string>
#include "compiler.h"
#include "constants.h"

ByteStream* Compiler::compileBody(AST_Function* function) {
    auto body = foldConstants(*(function->body));
    fullOutput->writeByte(0);
    int bodyFixup = fullOutput->getCurrentByteIndex();

    fullOutput->writeByte(function->locals.size());
    for (const auto& local : function->locals) {
        fullOutput->writeByte(1);
        fullOutput->writeByte(local.second.second);
    }

    for ( auto instruction : body ) {
        fullOutput->writeByte( instruction->instruction_code );

        if (instruction->instruction_code == constants::I32STORE || instruction->instruction_code == constants::I32LOAD) {
            fullOutput->writeByte(2); // alignment
            fullOutput->writeByte(instruction->parameter);
            continue;
        }

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
    fullOutput->fixUpByte(bodyFixup - 1, fullOutput->getCurrentByteIndex() - bodyFixup);

    return fullOutput;
}


std::vector<Instruction*> Compiler::foldConstants(std::vector<Instruction*> input) {
    std::vector<Instruction*> output;
    bool mergedSomethingInPreviousIteration = true;

    while( mergedSomethingInPreviousIteration ){
        mergedSomethingInPreviousIteration = false;
        output = std::vector<Instruction*>();

        for ( int i = 0; i < input.size(); ++i ) {
            Instruction* instruction = input[i];

            if ( instruction->type == InstructionType::CONST ) {
                // TODO: properly check if we're not at the end of the vector!
                Instruction* next     = input[ i + 1 ];
                Instruction* nextnext = input[ i + 2 ];

                if ( next->type == InstructionType::CONST && nextnext->type == InstructionType::CALCULATION ) {
                    // we can fold!
					Instruction *folded = new Instruction( InstructionType::CONST );
					folded->instruction_code = (int) instruction->instruction_code; // is i32.const!

                    if ( nextnext->instruction_code == constants::I32ADD ) {
					    folded->parameter = instruction->parameter + next->parameter;
                        
                        std::cout << "Compiler::foldConstants : Folded " << instruction->parameter << " + " << next->parameter << " to " << folded->parameter << std::endl;
                    } else if(nextnext->instruction_code == constants::I32SUB) {
                        folded->parameter = instruction->parameter - next->parameter;
                        std::cout << "Compiler::foldConstants : Folded " << instruction->parameter << " - " << next->parameter << " to " << folded->parameter << std::endl;
                    } else if (nextnext->instruction_code == constants::I32MUL) {
                        folded->parameter = instruction->parameter * next->parameter;
                        std::cout << "Compiler::foldConstants : Folded " << instruction->parameter << " * " << next->parameter << " to " << folded->parameter << std::endl;
                    } else if(nextnext->instruction_code == constants::F32ADD) {
                        folded->float_parameter = instruction->float_parameter + next->float_parameter;
                        std::cout << "Compiler::foldConstants : Folded " << instruction->float_parameter << " + " << next->float_parameter << " to " << folded->float_parameter << std::endl;
                    } else if(nextnext->instruction_code == constants::F32SUB) {
                        folded->float_parameter = instruction->float_parameter - next->float_parameter;
                        std::cout << "Compiler::foldConstants : Folded " << instruction->float_parameter << " - " << next->float_parameter << " to " << folded->float_parameter << std::endl;
                    } else if(nextnext->instruction_code == constants::F32MUL) {
                        folded->float_parameter = instruction->float_parameter * next->float_parameter;
                        std::cout << "Compiler::foldConstants : Folded " << instruction->float_parameter << " * " << next->float_parameter << " to " << folded->float_parameter << std::endl;
                    }
                    else {
                        std::cout << "Compiler::foldConstants : can't fold " << nextnext->instruction_code << std::endl;
                    }

					output.push_back( folded );

                    i += 2; // not 3, because the loop will do ++i at the end as well!
                    mergedSomethingInPreviousIteration = true;
                }
                else {
                    output.push_back( instruction );
                }
            }
            else {
                output.push_back( instruction );
            }
        }

        input = output; // prepare for next iteration
    }

    return output;
}

ByteStream *Compiler::compile() {
    int fixUpByte;
    bool exportFound = false;

    // magic number and version
    for (uint8_t byte : {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00}) {
        fullOutput->writeByte(byte);
    }

    // type section
    if (!functions.empty()) {
        fullOutput->writeByte(0x01);
        fullOutput->writeByte(0);
        fixUpByte = fullOutput->getCurrentByteIndex();
        writeTypeSection();
        fullOutput->fixUpByte(fixUpByte - 1, fullOutput->getCurrentByteIndex() - fixUpByte);
    }

    // import section
    // imported memory or functions will always be first in the list
    if (!memories.empty()) {
        if (memories[0]->isImported) {
            writeImportSection();
        }
    } else if (!functions.empty()) {
        if (functions[0]->isImported) {
            writeImportSection();
        }
    }

    // function section
    int importFunctions = 0;
    for (auto function : functions) {
        if (function->isImported) {
            importFunctions++;
        }
    }
    if ((functions.size() - importFunctions) > 0) {
        fullOutput->writeByte(0x03);
        fullOutput->writeByte(1 + (functions.size() - importFunctions));
        fullOutput->writeByte((functions.size() - importFunctions));
        for (auto func : functions) {
            if (func->isImported) {
                continue;
            }
            if (func->name.length() > 0) {
                exportFound = true;
            }
            for (int i = 0; i < functionTypes.size(); i++) {
                auto type = functionTypes[i];
                if (type[0] == func->parameters && type[1] == func->results) {
                    fullOutput->writeByte(i);
                    break;
                }
            }
        }
    }

    // memory section
    if (!memories.empty()) {
        if (!memories[0]->isImported) {
            exportFound = true;
            fullOutput->writeByte(0x05);
            fullOutput->writeByte(0);
            fixUpByte = fullOutput->getCurrentByteIndex();
            fullOutput->writeByte(memories.size());
            for (auto memory: memories) {
                if (memory->max_value > 0) {
                    fullOutput->writeByte(1); // limits flag for 2 limits
                } else {
                    fullOutput->writeByte(0); // no upper limit
                }
                fullOutput->writeByte(memory->initial_value);
                if (memory->max_value > 0) {
                    fullOutput->writeByte(memory->max_value);
                }
            }
            fullOutput->fixUpByte(fixUpByte - 1, fullOutput->getCurrentByteIndex() - fixUpByte);
        }
    }

    // export section
    if (exportFound) {
        fullOutput->writeByte(0x07);
        fullOutput->writeByte(0);
        fixUpByte = fullOutput->getCurrentByteIndex();
        writeExportSection();
        fullOutput->fixUpByte(fixUpByte - 1, fullOutput->getCurrentByteIndex() - fixUpByte);
    }

    // code section
    if ((functions.size() - importFunctions) > 0) {
        fullOutput->writeByte(0x0A);
        fullOutput->writeByte(0);
        fixUpByte = fullOutput->getCurrentByteIndex();
        fullOutput->writeByte(functions.size() - importFunctions);
        for (auto function : functions) {
            if (function->isImported) {
                continue;
            }
            compileBody(function);
        }
        fullOutput->fixUpByte(fixUpByte - 1, fullOutput->getCurrentByteIndex() - fixUpByte);
    }

    return fullOutput;
}

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

void Compiler::writeExportSection() {
    int memCount = 0;
    for (auto memory : memories) {
        if (memory->name.length() > 0 && !memory->isImported) {
            memCount++;
        }
    }
    int funcCount = 0;
    for (auto function : functions) {
        if (function->name.length() > 0 && !function->isImported) {
            funcCount++;
        }
    }
    fullOutput->writeByte(funcCount + memCount);
    for (int i = 0; i < memories.size(); i++) {
        if (memories[i]->name.length() == 0 || memories[i]->isImported) {
            continue;
        }
        fullOutput->writeByte(memories[i]->name.length());
        for (auto c : memories[i]->name) {
            fullOutput->writeByte(c);
        }
        fullOutput->writeByte(2); // type = memory
        fullOutput->writeByte(i); // index
    }
    for (int i = 0; i < functions.size(); i++) {
        if (functions[i]->name.length() == 0 || functions[i]->isImported) {
            continue;
        }
        fullOutput->writeByte(functions[i]->name.size());
        for (auto c : functions[i]->name) {
            fullOutput->writeByte(c);
        }
        fullOutput->writeByte(0); // type = function
        fullOutput->writeByte(i); // index
    }
}

void Compiler::writeImportSection() {
    fullOutput->writeByte(0x02);
    fullOutput->writeByte(0);
    int fixUpByte = fullOutput->getCurrentByteIndex();

    int exportMem = 0, exportFunc = 0;
    for (auto memory : memories) {
        if (memory->isImported) {
            exportMem++;
        }
    }
    for (auto function : functions) {
        if (function->isImported) {
            exportFunc++;
        }
    }
    fullOutput->writeByte(exportMem + exportFunc);
    for (int i = 0; i < exportMem; i++) {
        fullOutput->writeByte(memories[i]->importModule.length());
        for (auto c : memories[i]->importModule) {
            fullOutput->writeByte(c);
        }
        fullOutput->writeByte(memories[i]->importField.length());
        for (auto c : memories[i]->importField) {
            fullOutput->writeByte(c);
        }
        fullOutput->writeByte(2); // type = memory
        if (memories[i]->max_value > 0) {
            fullOutput->writeByte(1); // limits flag for 2 limits
        } else {
            fullOutput->writeByte(0); // no upper limit
        }
        fullOutput->writeByte(memories[i]->initial_value);
        if (memories[i]->max_value > 0) {
            fullOutput->writeByte(memories[i]->max_value);
        }
    }
    for (int i = 0; i < exportFunc; i++) {
        fullOutput->writeByte(functions[i]->importModule.length());
        for (auto c : functions[i]->importModule) {
            fullOutput->writeByte(c);
        }
        fullOutput->writeByte(functions[i]->importField.length());
        for (auto c : functions[i]->importField) {
            fullOutput->writeByte(c);
        }
        fullOutput->writeByte(0); // type = function
        for (int j = 0; j < functionTypes.size(); ++j) {
            auto type = functionTypes[j];
            if (type[0] == functions[i]->parameters && type[1] == functions[i]->results) {
                fullOutput->writeByte(j);
                break;
            }
        }
    }

    fullOutput->fixUpByte(fixUpByte - 1, fullOutput->getCurrentByteIndex() - fixUpByte);
}
