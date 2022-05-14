// Based on example on Toledo

#ifndef __COMPILER_H__
#define __COMPILER_H__

#include "bytestream.h"
#include "instruction.h"
#include "AST_Function.h"
#include <array>

class Compiler {
private:
    ByteStream* fullOutput = new ByteStream();
    std::vector<Instruction*> instructions;
    std::vector<AST_Function*> functions;
    std::vector<std::array<std::vector<VariableType>, 2>> functionTypes;

    std::vector<Instruction*> foldConstants(std::vector<Instruction*> input);
    ByteStream* compileBody(std::vector<Instruction*> body);
    void writeTypeSection();
    ByteStream* writeImportSection();
    ByteStream* writeTableSection();
    ByteStream* writeMemorySection();
    ByteStream* writeGlobalSection();
    void writeExportSection();
    ByteStream* writeStartSection();
    ByteStream* writeElementSection();
    ByteStream* writeCodeSection();
    ByteStream* writeDataSection();
    ByteStream* writeDataCountSection();

public:
	Compiler(std::vector<Instruction*> input) : instructions(input) {};
    Compiler(std::vector<AST_Function*> input) : functions(input) {};
    ~Compiler() {}
    
    ByteStream* compile();
};

#endif // __COMPILER_H__