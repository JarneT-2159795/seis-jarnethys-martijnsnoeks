#ifndef SEIS_JARNETHYS_MARTIJNSNOEKS_VARIABLE_H
#define SEIS_JARNETHYS_MARTIJNSNOEKS_VARIABLE_H

#include <variant>

#define float32_t float
#define float64_t double

typedef std::variant<int32_t, int64_t, float32_t, float64_t> Variable;

#endif //SEIS_JARNETHYS_MARTIJNSNOEKS_VARIABLE_H
