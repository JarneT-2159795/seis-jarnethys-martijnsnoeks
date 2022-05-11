#include "function.h"
#include <iostream>
#include <math.h>

Function::Function(std::vector<VariableType> parameterList, std::vector<VariableType> resultList, Stack *globalStack,
                   std::vector<Function> *moduleFunctions, std::vector<GlobalVariable> *moduleGlobals, std::vector<Memory> *moduleMemory)
            : params{ parameterList }, results{ resultList }, stack{ globalStack },
              functions{ moduleFunctions }, globals{ moduleGlobals }, memories{ moduleMemory } {}

void Function::setName(std::string functionName) {
    name = functionName;
}

std::string Function::getName() {
    return name;
}

void Function::addLocalVars(VariableType varType, int count) {
    for (int i = 0; i < count; ++i) {
        localVars.push_back(varType);
    }
}

void Function::setBody(std::vector<uint8_t> functionBody) {
    body = functionBody;
}

void Function::findJumps() {
    enum class TYPE { IF, BLOCK };
    auto origStack = stack;
    auto origGlobals = globals;
    auto origMemories = memories;
    Stack tempStack(*stack);
    std::vector<GlobalVariable> tempGlobals(*globals);
    std::vector<Memory> tempMemories(*memories);
    stack = &tempStack;
    globals = &tempGlobals;
    memories = &tempMemories;
    bs.readVector(body);
    uint8_t byte;

    std::vector<std::array<int, 2>> openIfJumps;
    std::vector<int> openBlockJumps;
    std::vector<TYPE> last;
    std::array<int, 6> forbidden = { BR, BR_IF, BLOCK, IF, ELSE, CALL };
    std::vector<int> dummy;
    while (!bs.atEnd() && bs.getRemainingByteCount() > 1) {
        byte = bs.readByte();
        switch (byte) {
            case BLOCK:
                {
                    openBlockJumps.push_back(bs.getCurrentByteIndex());
                    last.push_back(TYPE::BLOCK);
                    break;
                }
            case IF:
                {
                    stack->pop<int32_t>();
                    std::array<int, 2> a;
                    a[0] = bs.getCurrentByteIndex();
                    openIfJumps.push_back(a);
                    last.push_back(TYPE::IF);
                    break;
                }
            case ELSE:
                {
                    openIfJumps.back()[1] = bs.getCurrentByteIndex();
                    bs.seek(-1);
                    break;
                }
            case BLOCK_END:
                {
                    if (last.back() == TYPE::IF) {
                        std::array<int, 2> a;
                        a[0] = openIfJumps.back()[1];
                        a[1] = bs.getCurrentByteIndex() - 1;
                        ifJumps.insert(std::make_pair(openIfJumps.back()[0], a));
                        openIfJumps.pop_back();
                    } else if (last.back() == TYPE::BLOCK) {
                        blockJumps[openBlockJumps.back()] = bs.getCurrentByteIndex() - 1;
                        openBlockJumps.pop_back();
                    }
                    last.pop_back();
                    break;
                }

            default:
                break;
        }
        if (std::any_of(forbidden.begin(), forbidden.end(), [byte](int i){ return byte == i; })) {
            bs.seek(1);
        } else {
            performOperation(byte, dummy, dummy);
        }
    }
    stack = origStack;
    globals = origGlobals;
    memories = origMemories;
    jumpsCalculated = true;
}

void Function::operator()(int offset) {
    stackOffset = offset;
    for (auto par : localVars) {
        switch (par) {
            case VariableType::is_int32:
                stack->push(int32_t());
                break;
            case VariableType::is_int64:
                stack->push(int64_t());
                break;
            case VariableType::isfloat32_t:
                stack->push(float32_t());
                break;
            case VariableType::isfloat64_t:
                stack->push(float64_t());
                break;
        }
    }
    if (!jumpsCalculated) {
        findJumps();
    }
    
    std::vector<int> jumpStack;
    std::vector<int> ifStack;
    bs.readVector(body);
    uint8_t byte;
    while (!bs.atEnd()) {
        byte = bs.readByte();
        performOperation(byte, jumpStack, ifStack);
    }
    // remove input variables from stack
    stack->removeRange(stackOffset, stackOffset + params.size());
}

void Function::performOperation(uint8_t byte, std::vector<int> &jumpStack, std::vector<int> &ifStack) {
    switch (byte) {
        case BLOCK:
            {
                jumpStack.push_back(bs.getCurrentByteIndex());
                bs.seek(1);
                break;
            }
        case IF:
            {
                if (stack->pop<int32_t>()) {
                    ifStack.push_back(bs.getCurrentByteIndex());
                    bs.seek(1); // Result type
                } else {
                    bs.setByteIndex(ifJumps[bs.getCurrentByteIndex()][0]);
                }
                break;
            }
        case ELSE:
            {
                bs.setByteIndex(ifJumps[ifStack.back()][1]);
                break;
            }
        case CALL:
            {
                uint32_t funcIndex = bs.readUInt32();
                Function *func = &(*functions)[funcIndex];
                Function f = Function(func->params, func->results, func->stack, func->functions, func->globals, func->memories);
                f.setBody(func->body);
                f(stack->size() - functions->at(funcIndex).getParams().size());
                break;
            }
        case BR:
            {
                int depth = bs.readUInt32();
                bs.setByteIndex(blockJumps[jumpStack[jumpStack.size() - 1 - depth]]);
                for (int i = 0; i < depth + 1; ++i) {
                    jumpStack.pop_back();
                }
                break;
            }
        case BR_IF:
            {
                if (stack->pop<int32_t>()) {
                    int depth = bs.readUInt32();
                    bs.setByteIndex(blockJumps[jumpStack[jumpStack.size() - 1 - depth]]);
                    for (int i = 0; i < depth + 1; ++i) {
                        jumpStack.pop_back();
                    }
                } else {
                    bs.seek(1); // Break depth
                }
                break;
            }
        case DROP:
            stack->pop();
            break;
        case SELECT:
            {
                int32_t value = stack->pop<int32_t>();
                Variable var2 = stack->pop();
                Variable var1 = stack->pop();
                if (value) {
                    stack->push(var1);
                } else {
                    stack->push(var2);
                }
                break;
            }
        case LOCALGET:
            stack->push(stack->at(bs.readUInt32() + stackOffset));
            break;
        case LOCALSET:
            stack->at(bs.readUInt32() + stackOffset) = stack->pop();
            break;
        case LOCALTEE:
            {
                Variable var = stack->back();
                stack->at(bs.readUInt32() + stackOffset) = var;
                break;
            }
        case GLOBALGET:
            stack->push(globals->at(bs.readUInt32()).getVariable());
            break;
        case GLOBALSET:
            globals->at(bs.readUInt32()).setVariable(stack->pop());
            break;
        case I32CONST:
            stack->push(int32_t(bs.readInt32()));
            break;
        case I64CONST:
            stack->push(int64_t(bs.readInt64()));
            break;
        case F32CONST:
            stack->push(float32_t(bs.readFloat32()));
            break;
        case F64CONST:
            stack->push(float64_t(bs.readFloat64()));
            break;
        case I32EQZ:
            {
                int32_t var = stack->pop<int32_t>();
                stack->push(int32_t(var == 0));
                break;
            }
        case I32EQ:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 == var2));
                break;
            }
        case I32NE:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 != var2));
                break;
            }
        case I32LT_S:
        case I32LT_U:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 < var2));
                break;
            }
        case I32GT_S:
        case I32GT_U:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 > var2));
                break;
            }
        case I32LE_S:
        case I32LE_U:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 <= var2));
                break;
            }
        case I32GE_S:
        case I32GE_U:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 >= var2));
                break;
            }
        case F64LT:
            {
                float64_t var2 = stack->pop<float64_t>();
                float64_t var1 = stack->pop<float64_t>();
                stack->push(int32_t(var1 < var2));
                break;
            }
        case I32CLZ:
            {
                int32_t var = stack->pop<int32_t>();
                stack->push(int32_t(__builtin_clz(var)));
                break;
            }
        case I32CTZ:
            {
                int32_t var = stack->pop<int32_t>();
                stack->push(int32_t(__builtin_ctz(var)));
                break;
            }
        case I32POPCNT:
            {
                int32_t var = stack->pop<int32_t>();
                stack->push(int32_t(__builtin_popcount(var)));
                break;
            }
        case I32ADD:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 + var2));
                break;
            }
        case I32SUB:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 - var2));
                break;
            }
        case I32MUL:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 * var2));
                break;
            }
        case I32DIV_S:
        case I32DIV_U:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 / var2));
                break;
            }
        case I32REM_S:
        case I32REM_U:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 % var2));
                break;
            }
        case I32AND:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 & var2));
                break;
            }
        case I32OR:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 | var2));
                break;
            }
        case I32XOR:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 ^ var2));
                break;
            }
        case I32SHL:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 << var2));
                break;
            }
        case I32SHR_S:
        case I32SHR_U:
            {
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t(var1 >> var2));
                break;
            }
        case I32ROTL:
            {
                // from https://www.geeksforgeeks.org/rotate-bits-of-an-integer/
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t((var1 << var2) | (var1 >> (32 - var2))));
                break;
            }
        case I32ROTR:
            {
                // from https://www.geeksforgeeks.org/rotate-bits-of-an-integer/
                int32_t var2 = stack->pop<int32_t>();
                int32_t var1 = stack->pop<int32_t>();
                stack->push(int32_t((var1 >> var2) | (var1 << (32 - var2))));
                break;
            }
        case I64ADD:
            {
                int64_t var2 = stack->pop<int64_t>();
                int64_t var1 = stack->pop<int64_t>();
                stack->push(int64_t(var1 + var2));
                break;
            }
        case I64SUB:
            {
                int64_t var2 = stack->pop<int64_t>();
                int64_t var1 = stack->pop<int64_t>();
                stack->push(int64_t(var1 - var2));
                break;
            }
        case I64MUL:
            {
                int64_t var2 = stack->pop<int64_t>();
                int64_t var1 = stack->pop<int64_t>();
                stack->push(int64_t(var1 * var2));
                break;
            }
        case F32ADD:
            {
                float32_t var2 = stack->pop<float32_t>();
                float32_t var1 = stack->pop<float32_t>();
                stack->push(float32_t(var1 + var2));
                break;
            }
        case F32SUB:
            {
                float32_t var2 = stack->pop<float32_t>();
                float32_t var1 = stack->pop<float32_t>();
                stack->push(float32_t(var2 - var1));
                break;
            }
        case F32MUL:
            {
                float32_t var2 = stack->pop<float32_t>();
                float32_t var1 = stack->pop<float32_t>();
                stack->push(float32_t(var1 * var2));
                break;
            }
        case F64ADD:
            {
                float64_t var2 = stack->pop<float64_t>();
                float64_t var1 = stack->pop<float64_t>();
                stack->push(float64_t(var1 + var2));
                break;
            }
        case F64SUB:
            {
                float64_t var2 = stack->pop<float64_t>();
                float64_t var1 = stack->pop<float64_t>();
                stack->push(float64_t(var1 - var2));
                break;
            }
        case F64MUL:
            {
                float64_t var2 = stack->pop<float64_t>();
                float64_t var1 = stack->pop<float64_t>();
                stack->push(float64_t(var1 * var2));
                break;
            }
        case I32WRAP_I64:
            {
                int64_t var = stack->pop<int64_t>();
                stack->push(int32_t(var));
                break;
            }
        case I32TRUNC_F32_S:
        case I32TRUNC_F32_U:
        case I32REINTERPRET_F32:
            {
                float32_t var = stack->pop<float32_t>();
                stack->push(int32_t(std::trunc(var)));
                break;
            }
        case I32TRUNC_F64_S:
        case I32TRUNC_F64_U:
            {
                float64_t var = stack->pop<float64_t>();
                stack->push(int32_t(std::trunc(var)));
                break;
            }
        case I64EXTENDI32_S:
        case I64EXTENDI32_U:
            {
                int32_t var = stack->pop<int32_t>();
                stack->push(int64_t(var));
                break;
            }
        case I64TRUNC_F32_S:
        case I64TRUNC_F32_U:
            {
                float32_t var = stack->pop<float32_t>();
                stack->push(int64_t(std::trunc(var)));
                break;
            }
        case I64TRUNC_F64_S:
        case I64TRUNC_F64_U:
        case I64REINTERPRET_F64:
            {
                float64_t var = stack->pop<float64_t>();
                stack->push(int64_t(std::trunc(var)));
                break;
            }
        case MEMORY_BULK_OP:
            {
                // opcodes from https://github.com/WebAssembly/bulk-memory-operations/blob/master/proposals/bulk-memory-operations/Overview.md
                uint32_t operation = bs.readUInt32();
                switch (operation)
                {
                    case 0x0A: // mem.copy
                        {
                            int32_t len = stack->pop<int32_t>();
                            int32_t src = stack->pop<int32_t>();
                            int32_t dst = stack->pop<int32_t>();
                            if ((*memories)[dst].data()->size() < len) {
                                (*memories)[dst].data()->resize(len);
                            }
                            std::copy((*memories)[src].data()->begin(), (*memories)[src].data()->begin() + len, (*memories)[dst].data()->begin());
                            break;
                        }
                    case 0x0B: // mem.fill
                        {
                            uint32_t index = bs.readUInt32();
                            int32_t length = stack->pop<int32_t>();
                            int32_t value = stack->pop<int32_t>();
                            int32_t offset = stack->pop<int32_t>();
                            (*memories)[index].fill(offset, value, length);
                            break;
                        }
                }
            }
        case BLOCK_END:
            break;
        
        default:
            throw FunctionException("Invalid or unsupported instruction", byte);
        }
}
