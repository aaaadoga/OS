#include <cstdint>
class DiskManage
{
    public:
        DiskManage();
        ~DiskManage();
        void init();
        void read(int blockNum, char *buffer);
        void write(int blockNum, char *buffer);

};

struct FCB{
    uint16_t mode;
    char filename[10];
    uint16_t size;
    uint16_t addr[13];
};


extern uint64_t BITMAP[16];

struct Block{
    uint8_t data[40];
};

