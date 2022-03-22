#include "module.h"
#include <iostream>
#include <cstdarg>

Module::Module(std::string filepath) : bytestr{filepath} {
    bytestr.seek(8);    // Magic and version number
    uint8_t section;
    while (!bytestr.atEnd() && bytestr.getRemainingByteCount() > 1) {
        section = bytestr.readByte();
        switch (section) {
        case CUSTOM_SECTION:
            std::cout << "Custom section" << std::endl;
            break;
        case TYPE_SECTION:
            bytestr.seek(1); // Count function types
            readTypeSection(bytestr.readByte());
            break;
        case IMPORT_SECTION:
            readImportSection(bytestr.readByte());
            break;
        case FUNCTION_SECTION:
            readFunctionSection(bytestr.readByte());
            break;
        case TABLE_SECTION:
            readTableSection(bytestr.readByte());
            break;
        case MEMORY_SECTION:
            readMemorySection(bytestr.readByte());
            break;
        case GLOBAL_SECTION:
            readGlobalSection(bytestr.readByte());
            break;
        case EXPORT_SECTION:
            readExportSection(bytestr.readByte());
            break;
        case START_SECTION:
            readStartSection(bytestr.readByte());
            break;
        case ELEMENT_SECTION:
            readElementSection(bytestr.readByte());
            break;
        case CODE_SECTION:
            readCodeSection(bytestr.readByte());
            break;
        case DATA_SECTION:
            readDataSection(bytestr.readByte());
            break;
        case DATACOUNT_SECTION:
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
        auto var = &stack.at(stack.size() - i);
        switch (var->index())
        {
        case 0:
            std::cout << std::get<int32_t>(*var) << " ";
            break;
        case 1:
            std::cout << std::get<int64_t>(*var) << " ";
            break;
        case 2:
            std::cout << std::get<float32_t>(*var) << " ";
            break;
        case 3:
            std::cout << std::get<float64_t>(*var) << " ";
            break;
        default:
            break;
        }
    }
    std::cout << std::endl;
}

VariableType Module::getVarType(uint8_t type) {
    switch (type) {
            case INT32:
                return VariableType::is_int32;
                break;
            case INT64:
                return VariableType::is_int64;
                break;
            case FLOAT32:
                return VariableType::isfloat32_t;
                break;
            case FLOAT64:
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

void Module::readImportSection(int length) { std::cout << "import section" << std::endl; }

void Module::readFunctionSection(int length) {
    int numFunctions = bytestr.readByte();
    for (int i = 0; i < numFunctions; ++i) {
        int signature = 2 * bytestr.readByte();
        functions.emplace_back(Function(functionTypes[signature], functionTypes[signature + 1], &stack, &functions));
    }
}

void Module::readTableSection(int length) { std::cout << "table section" << std::endl; }

void Module::readMemorySection(int length) { std::cout << "memory section" << std::endl; }

void Module::readGlobalSection(int length) { std::cout << "global section" << std::endl; }

void Module::readExportSection(int length) {
    int exports = bytestr.readByte();
    for (int i = 0; i < exports; ++i) {
        auto name = bytestr.readASCIIString(bytestr.readByte());
        bytestr.seek(1); // TODO fix export kind
        functions[bytestr.readUInt32()].setName(name);
    }
}

void Module::readStartSection(int length) { std::cout << "start section" << std::endl; }

void Module::readElementSection(int length) { std::cout << "element section" << std::endl; }

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

void Module::readDataSection(int length) { std::cout << "data section" << std::endl; }

void Module::readDataCountSection(int length) { std::cout << "datacount section" << std::endl; }
