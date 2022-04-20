#include "function.h"
#include <cstdarg>
#include <iostream>
#include <algorithm>

Function::Function(std::vector<VariableType> paramaterList, std::vector<VariableType> resultList, std::vector<Variable> *globalStack, std::vector<Function> *moduleFunctions) 
            : params{ paramaterList }, results{ resultList }, stack{ globalStack }, functions{ moduleFunctions } {
};

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
    Stack tempStack(stack);
    stack = tempStack;
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
                    printf("Found block\n");
                    openBlockJumps.push_back(bs.getCurrentByteIndex());
                    last.push_back(TYPE::BLOCK);
                    break;
                }
            case IF:
                {
                    stack.pop();
                    printf("Found if\n");
                    std::array<int, 2> a;
                    a[0] = bs.getCurrentByteIndex();
                    openIfJumps.push_back(a);
                    last.push_back(TYPE::IF);
                    break;
                }
            case ELSE:
                {
                    printf("Found else\n");
                    openIfJumps.back()[1] = bs.getCurrentByteIndex();
                    bs.seek(-1);
                    break;
                }
            case END:
                {
                    if (last.back() == TYPE::IF) {
                        printf("Closed if\n");
                        std::array<int, 2> a;
                        a[0] = openIfJumps.back()[1];
                        a[1] = bs.getCurrentByteIndex();
                        ifJumps.insert(std::make_pair(openIfJumps.back()[0], a));
                        openIfJumps.pop_back();
                    } else if (last.back() == TYPE::BLOCK) {
                        printf("Closed block\n");
                        blockJumps[openBlockJumps.back()] = bs.getCurrentByteIndex();
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
    jumpsCalculated = true;
}

void Function::operator()(int offset) {
    stackOffset = offset;
    for (auto par : localVars) {
        switch (par) {
            case VariableType::is_int32:
                stack.push(int32_t());
                break;
            case VariableType::is_int64:
                stack.push(int64_t());
                break;
            case VariableType::isfloat32_t:
                stack.push(float32_t());
                break;
            case VariableType::isfloat64_t:
                stack.push(float64_t());
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
                if (std::get<int32_t>(stack.pop())) {
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
                functions->at(funcIndex).operator()(stack.size() - functions->at(funcIndex).getParams().size());
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
                if (std::get<int32_t>(stack.pop())) {
                    int depth = bs.readUInt32();
                    bs.setByteIndex(blockJumps[jumpStack[jumpStack.size() - 1 - depth]]);
                    for (int i = 0; i < depth + 1; ++i) {
                        jumpStack.pop_back();
                    }
                } else {
                    bs.seek(1); // Break depth
                }
                stack.pop();
                break;
            }
        case LOCALGET:
            stack.push(stack.at(bs.readUInt32() + stackOffset));
            break;
        case LOCALSET:
            stack.at(bs.readUInt32() + stackOffset) = stack.pop();

            break;
        case I32CONST:
            stack.push(int32_t(bs.readInt32()));
            break;
        case I64CONST:
            stack.push(int64_t(bs.readInt64()));
            break;
        case F32CONST:
            stack.push(float32_t(bs.readFloat32()));
            break;
        case F64CONST:
            stack.push(float64_t(bs.readFloat64()));
            break;
        case I32EQZ:
            {
                int32_t var = std::get<int32_t>(stack.pop());
                stack.push(int32_t(var == 0));
                break;
            }
        case I32EQ:
            {
                int32_t var2 = std::get<int32_t>(stack.pop());
                int32_t var1 = std::get<int32_t>(stack.pop());
                stack.push(int32_t(var1 == var2));
                break;
            }
        case I32LT_S:
            {
                int32_t var2 = std::get<int32_t>(stack.pop());
                int32_t var1 = std::get<int32_t>(stack.pop());
                stack.push(int32_t(var1 < var2));
                break;
            }
        case F64LT:
            {
                float64_t var2 = std::get<float64_t>(stack.pop());
                
                float64_t var1 = std::get<float64_t>(stack.pop());
               
                stack.push(int32_t(var1 < var2));
                break;
            }
        case I32ADD:
            {
                int32_t var2 = std::get<int32_t>(stack.pop());
                
                int32_t var1 = std::get<int32_t>(stack.pop());
               
                stack.push(int32_t(var1 + var2));
                break;
            }
        case I32SUB:
            {
                int32_t var2 = std::get<int32_t>(stack.pop());
                
                int32_t var1 = std::get<int32_t>(stack.pop());
              
                stack.push(int32_t(var1 - var2));
                break;
            }
        case I32MUL:
            {
                int32_t var2 = std::get<int32_t>(stack.pop());
               
                int32_t var1 = std::get<int32_t>(stack.pop());
                ;
                stack.push(int32_t(var1 * var2));
                break;
            }
        case I64ADD:
            {
                int64_t var2 = std::get<int64_t>(stack.pop());
              
                int64_t var1 = std::get<int64_t>(stack.pop());
                
                stack.push(int64_t(var1 + var2));
                break;
            }
        case I64SUB:
            {
                int64_t var2 = std::get<int64_t>(stack.pop());
                
                int64_t var1 = std::get<int64_t>(stack.pop());
               
                stack.push(int64_t(var1 - var2));
                break;
            }
        case I64MUL:
            {
                int64_t var2 = std::get<int64_t>(stack.pop());
              
                int64_t var1 = std::get<int64_t>(stack.pop());
              
                stack.push(int64_t(var1 * var2));
                break;
            }
        case F32ADD:
            {
                float32_t var2 = std::get<float32_t>(stack.pop());
               
                float32_t var1 = std::get<float32_t>(stack.pop());
                stack.push(float32_t(var1 + var2));
                break;
            }
        case F32SUB:
            {
                float32_t var2 = std::get<float32_t>(stack.pop());
                
                float32_t var1 = std::get<float32_t>(stack.pop());
                
                stack.push(float32_t(var2 - var1));
                break;
            }
        case F32MUL:
            {
                float32_t var2 = std::get<float32_t>(stack.pop());
               
                float32_t var1 = std::get<float32_t>(stack.pop());
              
                stack.push(float32_t(var1 * var2));
                break;
            }
        case F64ADD:
            {
                float64_t var2 = std::get<float64_t>(stack.pop());
               
                float64_t var1 = std::get<float64_t>(stack.pop());
               
                stack.push(float64_t(var1 + var2));
                break;
            }
        case F64SUB:
            {
                float64_t var2 = std::get<float64_t>(stack.pop());
                
                float64_t var1 = std::get<float64_t>(stack.pop());
                
                stack.push(float64_t(var1 - var2));
                break;
            }
        case F64MUL:
            {
                float64_t var2 = std::get<float64_t>(stack.pop());
                
                float64_t var1 = std::get<float64_t>(stack.pop());
            
                stack.push(float64_t(var1 * var2));
                break;
            }
        case END:
            break;
        
        default:
            throw FunctionException("Invalid or unsupported instuction", byte);
            break;
        }
}