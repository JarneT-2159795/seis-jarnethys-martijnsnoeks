#include <string>

enum class VariableType { is_int32, is_int64, is_float32, is_float64 };

class Variable {
public:
    Variable(VariableType varType);
    Variable(VariableType varType, size_t value);
    size_t get();
    void set(size_t value);

private:
    VariableType vt;
    union {
        int32_t i32val;
        int64_t i64val;
        _Float32 f32val;
        _Float64 f64val;
    } val;
};