#ifndef SEIS_JARNETHYS_MARTIJNSNOEKS_MEMORY_H
#define SEIS_JARNETHYS_MARTIJNSNOEKS_MEMORY_H

#include <string>
#include <vector>
#include "Variable.h"

class Memory {
public:
    Memory(uint32_t init_size);
    Memory(uint32_t init_size, uint32_t max_size);

    void setMemory(int index, Variable var);
    Variable getMemory(int index);

    void setName(std::string memName) { this->name = memName; }
    std::string getName() { return name; };

    void fill(uint32_t offset, int32_t value, uint32_t length);
    std::vector<Variable>* data() { return &memory; }

private:
    std::string name;
    uint32_t initial;
    uint32_t maximum;
    std::vector<Variable> memory;
};


#endif
