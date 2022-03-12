#include "module.h"
#include <iostream>

Module::Module(std::string filepath) : bytestr{filepath} {
    for (int i = 0; i < 8; ++i) {   // Magic and version number
        bytestr.readByte();
    }
    uint8_t section;
    while (!bytestr.atEnd()) {
        section = bytestr.readByte();
        switch (section) {
        case 0x00:
            std::cout << "Custom section at byte " << std::hex << bytestr.getCurrentByteIndex() << std::dec << std::endl;
            break;
        case 0x01:
            bytestr.readByte(); // Count function types
            readTypeSection(bytestr.readByte());
            break;
        case 0x02:
            readImportSection(bytestr.readByte());
            break;
        case 0x03:
            readFunctionSection(bytestr.readByte());
            break;
        case 0x04:
            readTableSection(bytestr.readByte());
            break;
        case 0x05:
            readMemorySection(bytestr.readByte());
            break;
        case 0x06:
            readGlobalSection(bytestr.readByte());
            break;
        case 0x07:
            readExportSection(bytestr.readByte());
            break;
        case 0x08:
            readStartSection(bytestr.readByte());
            break;
        case 0x09:
            readElementSection(bytestr.readByte());
            break;
        case 0x0A:
            readCodeSection(bytestr.readByte());
            break;
        case 0x0B:
            readDataSection(bytestr.readByte());
            break;
        case 0x0C:
            readDataCountSection(bytestr.readByte());
            break;
        default:
            throw ModuleException("Invalid file: not a valid section code", bytestr.getCurrentByteIndex());
            break;
        }
    }
}

std::vector<Function> Module::getFunctions() {
    return functions;
}

VariableType Module::getVarType(uint8_t type) {
    switch (type) {
            case 0x7F:
                return VariableType::is_int32;
                break;
            case 0x7E:
                return VariableType::is_int64;
                break;
            case 0x7D:
                return VariableType::is_float32;
                break;
            case 0x7C:
                return VariableType::is_float64;
                break;
            default:
                throw ModuleException("Invalid file: not a valid parameter type", bytestr.getCurrentByteIndex());
                break;
            }
}

void Module::readTypeSection(int length) {
    while(bytestr.readByte() == 0x60) {  // Read a function type
        // Read the type of function parameters
        int numParams = bytestr.readByte();
        std::vector<Variable> params;
        for (int i = 0; i < numParams; ++i) {
            params.push_back(Variable(getVarType(bytestr.readByte())));
        }

        // Read the type of function results
        int numResults = bytestr.readByte();
        std::vector<Variable> results;
        for (int i = 0; i < numResults; ++i) {
            results.push_back(Variable(getVarType(bytestr.readByte())));
        }
        functions.push_back(Function(params, results));
    }
    bytestr.seek(-1);
}

void Module::readImportSection(int length) {}

void Module::readFunctionSection(int length) {
    int numFunctions = bytestr.readByte();
    for (int i = 0; i < numFunctions; ++i) {
        int signature = bytestr.readByte();
    }
}

void Module::readTableSection(int length) {}

void Module::readMemorySection(int length) {}

void Module::readGlobalSection(int length) {}

void Module::readExportSection(int length) {
    int exports = bytestr.readByte();
    for (int i = 0; i < exports; ++i) {
        auto var = bytestr.readASCIIString(bytestr.readByte());
        functions[i].setName(var);
        bytestr.seek(2); // TODO fix export kind and func index
    }
}

void Module::readStartSection(int length) {}

void Module::readElementSection(int length) {}

void Module::readCodeSection(int length) {
    int numFunctions = bytestr.readByte();
    for (int i = 0; i < numFunctions; ++i) {
        int bodySize = bytestr.readByte();
        int localVarTypes = bytestr.readByte();
        for (int j = 0; j < localVarTypes; ++j) {
            int typeCount = bytestr.readByte();
            functions[i].addLocalVars(getVarType(bytestr.readByte()), typeCount);
        }

        uint8_t byte = bytestr.readByte();
        std::vector<uint8_t> body;
        std:: cout << std::hex;
        while (byte != 0x0B) {
            body.push_back(byte);
            byte = bytestr.readByte();
        }
    }
}

void Module::readDataSection(int length) {}

void Module::readDataCountSection(int length) {}
