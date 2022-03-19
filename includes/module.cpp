#include "module.h"
#include <iostream>
#include <cstdarg>

Module::Module(std::string filepath) : bytestr{filepath} {
    bytestr.seek(8);    // Magic and version number
    uint8_t section;
    while (!bytestr.atEnd() && bytestr.getRemainingByteCount() > 1) {
        section = bytestr.readByte();
        switch (section) {
        case 0x00:
            std::cout << "Custom section" << std::endl;
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

void Module::operator()(std::string name, std::vector<Variable> vars) {
    for (auto func : functions) {
        if (func.getName() == name) {
            stack.insert(stack.end(), vars.begin(), vars.end());
            func(stack.size() - func.getParams().size());
            return;
        }
    }
    throw ModuleException("Function '" + name + "' not found");
}

void Module::printVariables(int amount) {
    for (int i = amount; i > 0; --i) {
        auto var = &stack.at(stack.size() - amount);
        switch (var->index())
        {
        case 0:
            std::cout << std::get<int32_t>(*var) << std::endl;
            break;
        case 1:
            std::cout << std::get<int64_t>(*var) << std::endl;
            break;
        case 2:
            std::cout << std::get<float32_t>(*var) << std::endl;
            break;
        case 3:
            std::cout << std::get<float64_t>(*var) << std::endl;
            break;
        default:
            break;
        }
    }
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
                return VariableType::isfloat32_t;
                break;
            case 0x7C:
                return VariableType::isfloat64_t;
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
        std::vector<VariableType> params;
        for (int i = 0; i < numParams; ++i) {
            params.push_back(getVarType(bytestr.readByte()));
        }

        // Read the type of function results
        int numResults = bytestr.readByte();
        std::vector<VariableType> results;
        for (int i = 0; i < numResults; ++i) {
            results.push_back(getVarType(bytestr.readByte()));
        }
        functionTypes.push_back(params);
        functionTypes.push_back(results);
    }
    bytestr.seek(-1);
}

void Module::readImportSection(int length) {}

void Module::readFunctionSection(int length) {
    int numFunctions = bytestr.readByte();
    for (int i = 0; i < numFunctions; ++i) {
        int signature = bytestr.readByte();
        functions.emplace_back(Function(functionTypes[signature], functionTypes[signature + 1], &stack));
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
        while (byte != 0x0B) {
            body.push_back(byte);
            byte = bytestr.readByte();
        }
        functions[i].setBody(body);
    }
}

void Module::readDataSection(int length) {}

void Module::readDataCountSection(int length) {}
