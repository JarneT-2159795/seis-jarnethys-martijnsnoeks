#include "bytestream.h"
#include "function.h"
#include <stack>
#include <exception>

class Module {
public:
    Module(std::string filepath);
    std::vector<Function> getFunctions();

private:
    ByteStream bytestr;
    std::stack<Variable> stack;
    std::vector<Function> functions;

    VariableType getVarType(uint8_t type);

    void readTypeSection(int length);
    void readImportSection(int length);
    void readFunctionSection(int length);
    void readTableSection(int length);
    void readMemorySection(int length);
    void readGlobalSection(int length);
    void readExportSection(int length);
    void readStartSection(int length);
    void readElementSection(int length);
    void readCodeSection(int length);
    void readDataSection(int length);
    void readDataCountSection(int length);
};

struct ModuleException : public std::exception
{
   std::string s;
   ModuleException(std::string ss, int byte) : s(ss + " at byte " + std::to_string((int)byte)) {}
   ~ModuleException() throw () {}
   const char* what() const throw() { return s.c_str(); }
};
