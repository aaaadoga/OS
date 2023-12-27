#include <cstdint>

struct memory_block {
    uint8_t data[40];
};

extern memory_block memory[64];