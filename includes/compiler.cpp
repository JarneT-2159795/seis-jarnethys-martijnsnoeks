// Based on example on Toledo

#include <iostream>
#include <iomanip> // std::setw(), std::setfill()
#include <vector>
#include <string>

#include "compiler.h"

Compiler::Compiler(std::vector<Instruction*> input)
{
	// TODO: now we just make a big bytestream of size 2000 bytes filled with \0
	// this should be better (e.g., grow dynamically as we compile more in increments!)
    this->output = new ByteStream( std::vector<unsigned char>(2000, '\0') );
    
    this->instructions = input;
}

ByteStream* Compiler::compile()
{
    // NOTE: comment this out to see "original" compiler output
    this->instructions = this->foldConstants( this->instructions );

    for ( auto instruction : this->instructions ) {
        this->output->writeUInt32( instruction->instruction_code );

        if ( instruction->type == InstructionType::CONST || instruction->type == InstructionType::INSTRUCTION_WITH_PARAMETER ) {
            this->output->writeUInt32( instruction->parameter );
        }
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

    return this->output;
}


std::vector<Instruction*> Compiler::foldConstants(std::vector<Instruction*> input)
{
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
                    if ( nextnext->instruction_code == (int) InstructionNumber::Operation::I32ADD ) {
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