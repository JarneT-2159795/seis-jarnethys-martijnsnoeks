#include "../includes/module.h"
#include <iostream>
#include <time.h>
#include "../includes/lexer.h"
#include "../includes/parser.h"
#include "../includes/compiler.h"

extern "C" {
    int useModule(uint8_t *data, int size, char* name, int32_t *int32Input, int64_t *int64Input, float32_t *float32Input, float64_t *float64Input,
                    int32_t *int32Output, int64_t *int64Output, float32_t *float32Output, float64_t *float64Output);
    int compile(uint8_t *data, int size, char *name, int32_t *int32Input, int64_t *int64Input, float32_t *float32Input, float64_t *float64Input,
                    int32_t *int32Output, int64_t *int64Output, float32_t *float32Output, float64_t *float64Output, uint8_t *output, int *outputSize);
    int randomInt() {
        srand(time(0));
        return rand();
    }
}

int compile(uint8_t *data, int size, char *name, int32_t *int32Input, int64_t *int64Input, float32_t *float32Input, float64_t *float64Input,
                int32_t *int32Output, int64_t *int64Output, float32_t *float32Output, float64_t *float64Output, uint8_t *output, int *outputSize) {
    std::vector<uint8_t> chars;
    for (int i = 0; i < size; i++) {
        chars.push_back(data[i]);
    }
	Lexer lexer = Lexer{chars};

    int err = lexer.lex();

    Parser parser = Parser(&lexer);

    parser.parseProper();
    
    Compiler compiler = Compiler(parser.getFunctions(), parser.getMemories(), parser.getDatas());
    auto compiledOutput = compiler.compile();

    uint8_t *outputbuffer = (uint8_t *)malloc(compiledOutput->getTotalByteCount());
    compiledOutput->setByteIndex(0);
    for (int i = 0; i < compiledOutput->getTotalByteCount(); i++) {
        output[i] = compiledOutput->readByte();
    }
    *outputSize = compiledOutput->getTotalByteCount();

    useModule(compiledOutput->getBuffer(), compiledOutput->getTotalByteCount(), name, int32Input, int64Input, float32Input, float64Input,
                int32Output, int64Output, float32Output, float64Output);

    return 0;
}

int useModule(uint8_t *data, int size, char *name, int32_t *int32Input, int64_t *int64Input, float32_t *float32Input, float64_t *float64Input,
                int32_t *int32Output, int64_t *int64Output, float32_t *float32Output, float64_t *float64Output) {
    if (size < 1) {
        std::cout << "Error: No data received" << std::endl;
        return -1;
    }

    std::string nameStr(name);
    if (nameStr == "") {
        std::cout << "Error: No name received" << std::endl;
        return -2;
    }

    int choice = -1;
    ByteStream bs;
    Module module{data, size};
    auto funcs = module.getFunctions();
    for (int i = 0; i < funcs.size(); i++) {
        if (funcs[i].getName() == nameStr) {
            choice = i;
            break;
        }
    }
    if (choice == -1) {
        std::cout << "Error: No function with name " << nameStr << " found" << std::endl;
        return -3;
    }

    Stack vars;
    int i32Count, i64Count, f32Count, f64Count;
    for (int i = 0; i < funcs[choice].getParams().size(); i++) {
        switch (funcs[choice].getParams()[i]) {
            case VariableType::is_int32:
                vars.push(int32Input[i32Count]);
                i32Count++;
                break;
            case VariableType::is_int64:
                vars.push(int64Input[i64Count]);
                i64Count++;
                break;
            case VariableType::isfloat32_t:
                vars.push(float32Input[f32Count]);
                f32Count++;
                break;
            case VariableType::isfloat64_t:
                vars.push(float64Input[f64Count]);
                f64Count++;
                break;
        }
    }

    module(name, vars);
    auto results = module.getResults(funcs[choice].getResults().size());
    for (int i = 0; i < results.size(); i++) {
        switch (funcs[choice].getResults()[i]) {
            case VariableType::is_int32:
                int32Output[i] = std::get<int32_t>(results[i]);
                std::cout << "int32Output[" << i << "] = " << int32Output[i] << std::endl;
                break;
            case VariableType::is_int64:
                int64Output[i] = std::get<int64_t>(results[i]);
                break;
            case VariableType::isfloat32_t:
                float32Output[i] = std::get<float32_t>(results[i]);
                break;
            case VariableType::isfloat64_t:
                float64Output[i] = std::get<float64_t>(results[i]);
                break;
        }
    }
    return 0;
}
