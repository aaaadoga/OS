#include <cstdint>

struct MemoryBlock {
    uint8_t data[40];
};

extern uint8_t BITMAP;

extern MemoryBlock memory[64]; 

int malloc_memory(uint16_t *addr );
