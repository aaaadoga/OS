#include <cstdint>
#include <fstream>
#include <inter_process.h>


struct inode {
  uint16_t mode; // dir or file
  uint16_t size;
  uint16_t addr[18];
};

struct FileBlock {
  uint8_t data[40];
};

struct dir_item {
  char name[8];
  uint16_t addr;
};

struct DirBlock {
  dir_item data[4];
};

int format();

int createFile(uint16_t dir_addr, const char *name, uint16_t *file_addr,
               char *buffer, int size); //

int createDir(uint16_t dir_addr, char *name, uint16_t *child_dir_addr); //

int getFreeBlock(uint16_t *addr,
                 uint16_t blockNum); // addr为地址数组，长度为blockNum///

int deleteBlock(uint16_t *addr,uint16_t blockNum); // 删除block///

int deleteInode(uint16_t dir_addr, uint16_t addr); // 删除inode///

int loadFile(uint16_t addr, char *buffer); //读内容


int readDir(uint16_t addr, FCB_P *buffer, uint16_t *size); //读目录

int readBlock(uint16_t addr,FileBlock * buffer);

int loadToMemory(uint8_t index,uint16_t inode_addr);

extern uint64_t _BITMAP[16];

extern FileBlock fileBlocks[1024];

int show();