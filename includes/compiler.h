// Based on example on Toledo

#ifndef __COMPILER_H__
#define __COMPILER_H__

#include "bytestream.h"
#include "instruction.h"

class Compiler
{
private:
    ByteStream* output;
    std::vector<Instruction*> instructions;

    std::vector<Instruction*> foldConstants(std::vector<Instruction*> input);

public:
	Compiler(std::vector<Instruction*> input);
    ~Compiler() {}
    
    ByteStream* compile();
};

#endif // __COMPILER_H__