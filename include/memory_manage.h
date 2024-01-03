#include <cstdint>

struct MemoryBlock {
  uint8_t data[40];
};

struct PageTable {
  uint8_t page_addr[18];//块号0~63
  bool is_valid[18];//是否在内存中
  bool is_dirty[18];//是否被修改
  bool is_full;//内存是否已满
  uint16_t inode_addr[18];//inode地址
  uint8_t point = 0;//0~7
  uint8_t size = 0;//0~18
};

extern PageTable page_table[8];

extern uint8_t BITMAP;

extern MemoryBlock memory[64];

int malloc_memory(uint16_t *index);

int free_memory(uint16_t index);

int request_page(
    uint8_t index, uint8_t page_id,
    MemoryBlock *data); // index: 0-7, page_id: 0-17, data: 40 bytes

int load_file(uint8_t index, uint8_t size, uint16_t *inode_addr);//index: 0-7, size: 1-18, inode_addr:数组