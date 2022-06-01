// Based on example on Toledo

#ifndef __COMPILER_H__
#define __COMPILER_H__

#include "bytestream.h"
#include "instruction.h"
#include "AST_Types.h"
#include <array>

class Compiler {
private:
    ByteStream* fullOutput = new ByteStream();
    std::vector<Instruction*> instructions;
    std::vector<AST_Function*> functions;
    std::vector<AST_Memory*> memories;
    std::vector<AST_Data*> datas;
    std::vector<std::array<std::vector<VariableType>, 2>> functionTypes;

    std::vector<Instruction*> foldConstants(std::vector<Instruction*> input);
    ByteStream* compileBody(AST_Function* function);
    void writeTypeSection();
    void writeImportSection();
    void writeExportSection();

public:
    Compiler(std::vector<AST_Function*> funcs, std::vector<AST_Memory*> mems, std::vector<AST_Data*> data) : functions(funcs), memories(mems), datas(data) {};

    ByteStream* compile();
    void writeFile(std::string filepath) { fullOutput->writeFile(filepath); };
};

#endif // __COMPILER_H__