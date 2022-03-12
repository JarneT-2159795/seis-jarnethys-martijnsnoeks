#include "variable.h"

Variable::Variable(VariableType varType, size_t value) : vt{varType} {
    switch (varType) {
        case VariableType::is_int32:
            val.i32val = value;
            break;
        case VariableType::is_int64:
            val.i64val = value;
            break;
        case VariableType::is_float32:
            val.f32val = value;
            break;
        case VariableType::is_float64:
            val.f64val = value;
            break;
        }
}

Variable::Variable(VariableType varType) : Variable(varType, 0) {
};

VariableType Variable::getType() {
    switch (vt) {
        case VariableType::is_int32:
            return VariableType::is_int32;
        case VariableType::is_int64:
            return VariableType::is_int64;
        case VariableType::is_float32:
            return VariableType::is_float32;
        case VariableType::is_float64:
            return VariableType::is_float64;
        default:
            return VariableType::is_int64;
    }
}

size_t Variable::get() {
    switch (vt) {
        case VariableType::is_int32:
            return val.i32val;
        case VariableType::is_int64:
            return val.i64val;
        case VariableType::is_float32:
            return val.f32val;
        case VariableType::is_float64:
            return val.f64val;
        default:
            return 0;
        }
}

void Variable::set(size_t value) {
    switch (vt) {
        case VariableType::is_int32:
            val.i32val = value;
            break;
        case VariableType::is_int64:
            val.i64val = value;
            break;
        case VariableType::is_float32:
            val.f32val = value;
            break;
        case VariableType::is_float64:
            val.f64val = value;
            break;
    }
}
