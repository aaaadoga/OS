#include <cstdint>
#include <fstream>
class DiskManage {
public:
  DiskManage();
  ~DiskManage();
  void init();
  void read(int blockNum, char *buffer);
  void write(int blockNum, char *buffer);
};

struct FCB {
  uint16_t mode;
  uint16_t size;
  char name[8];
  uint16_t addr;
};

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

int loadFile(uint16_t addr, char *buffer); //


int readDir(uint16_t addr, FCB *buffer, uint16_t *size); //

int swapBlock(uint16_t addr,FileBlock * buffer);

extern uint64_t BITMAP[16];

extern FileBlock fileBlocks[1024];