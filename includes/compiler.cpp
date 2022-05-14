// Based on example on Toledo

#include <iostream>
#include <iomanip> // std::setw(), std::setfill()
#include <vector>
#include <string>

#include "compiler.h"
#include "constants.h"

ByteStream* Compiler::compileBody(std::vector<Instruction*> body) {
    fullOutput->writeByte(0);
    int bodyFixup = fullOutput->getCurrentByteIndex();
    fullOutput->writeByte(0); // TODO local variables

    // NOTE: comment this out to see "original" compiler output
    //body = this->foldConstants( body );

    for ( auto instruction : body ) {
        fullOutput->writeByte( instruction->instruction_code );

        if ( instruction->type == InstructionType::CONST || instruction->type == InstructionType::INSTRUCTION_WITH_PARAMETER ) {
            fullOutput->writeUInt32( instruction->parameter );
        }
    }
    fullOutput->fixUpByte(bodyFixup - 1, fullOutput->getCurrentByteIndex() - bodyFixup);
    

    //int writtenByteCount = output->getCurrentByteIndex() - 1; // -1 because byteIndex is ready to write a new byte now!

	//output->setByteIndex(0);

	//std::cout << "Nr bytes written " << writtenByteCount << std::endl;
/*
	int c = 0;
	while( c <= writtenByteCount ) { 
		unsigned char byte = output->readByte();

		std::cout << std::hex << std::setw(2) << std::setfill('0') << (int) byte << std::dec << std::endl;

		++c;
	}
*/
    return fullOutput;
}


std::vector<Instruction*> Compiler::foldConstants(std::vector<Instruction*> input) {
    std::vector<Instruction*> output;

    // Logic: we want to fold constants and their operations here
    // For example:
    //      i32.const 15
    //      i32.const 20
    //      i32.add
    // should just be replaced with:
    //      i32.const 35
    // TODO: support more than just i32.add

    // general logic: when we encounter an add, look back to the previous two instructions
    // if those are of type CONST, we can fold!

    // practical logic: 
    // if we encounter a const, look 2 ahead. If it's another const and an add, merge, skip them.
    
    // finally, we want to be able to "collapse" this:
    //      i32.const 15
    //      i32.const 20
    //      i32.add
    //      i32.const 30
    //      i32.add
    // should finally become:
    //      i32.const 65
    // we COULD do this in 1 pass, but that would mean more complex code
    // below, we do this in multiple passes, were we first collapse the first constant add,
    // which then becomes
    //      i32.const 35
    //      i32.const 30
    //      i32.add
    // which can then be collapsed in the second iteration

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

                    // TODO: support other things besides i32.add!!!
                    if ( nextnext->instruction_code == (int) constants::I32ADD ) {
					    folded->parameter = instruction->parameter + next->parameter;
                        
                        std::cout << "Compiler::foldConstants : Folded " << instruction->parameter << " + " << next->parameter << " to " << folded->parameter << std::endl;
                    }
                    else {
                        std::cout << "Compiler::foldConstants : can't fold for other stuff than i32.add yet! " << nextnext->instruction_code << std::endl;
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
    // magic number and version
    for (uint8_t byte : {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00}) {
        fullOutput->writeByte(byte);
    }

    // type section
    fullOutput->writeByte(0x01);
    fullOutput->writeByte(0);
    int fixUpByte(fullOutput->getCurrentByteIndex());
    writeTypeSection();
    fullOutput->fixUpByte(fixUpByte - 1, fullOutput->getCurrentByteIndex() - fixUpByte);

    // function section
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

    // export section
    fullOutput->writeByte(0x07);
    fullOutput->writeByte(0);
    fixUpByte = fullOutput->getCurrentByteIndex();
    writeExportSection();
    fullOutput->fixUpByte(fixUpByte - 1, fullOutput->getCurrentByteIndex() - fixUpByte);

    // code section
    fullOutput->writeByte(0x0A);
    fullOutput->writeByte(0);
    fixUpByte = fullOutput->getCurrentByteIndex();
    fullOutput->writeByte(functions.size());
    for (auto function : functions) {
        compileBody(*function->body);
    }
    fullOutput->fixUpByte(fixUpByte - 1, fullOutput->getCurrentByteIndex() - fixUpByte);

    fullOutput->writeFile("./output.wasm");
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
