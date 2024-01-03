#include <cstring>
#include <disk_manage.h>
#include <iostream>

uint64_t BITMAP[16] = {0};
FileBlock fileBlocks[1024];
std::fstream file("disk.dat", std::ios::in | std::ios::out | std::ios::binary);

int main() {
  // 判断文件是否存在
  if (!file) {
    std::cout << "file not exist" << std::endl;
    // 创建文件
    file.open("disk.dat", std::ios::out | std::ios::binary);
    // 格式化磁盘
    std::cout << "formating" << std::endl;
    format();
    file.close();
    file.open("disk.dat", std::ios::in | std::ios::out | std::ios::binary);
  }
  // 读取磁盘,检查磁盘完整性
  file.seekg(0, std::ios::beg);
  inode root;
  file.read(reinterpret_cast<char *>(&root), sizeof(FileBlock));
  file.seekg(1024 * sizeof(FileBlock), std::ios::beg);
  file.read(reinterpret_cast<char *>(&BITMAP), sizeof(uint64_t));

  if ((BITMAP[0] & 0x8000000000000000) == 0 || (root.mode & 0x8000) == 0) {
    std::cout << "disk error,formating" << std::endl;
    format();
    file.close();
    file.open("disk.dat", std::ios::in | std::ios::out | std::ios::binary);
  }
  // 检查完毕
  std::cout << "disk ok" << std::endl;




 
}

int format() {
  // 初始化位图
  memset(BITMAP, 0, 16 * sizeof(uint64_t));
  // 初始化Block
  memset(fileBlocks, 0, 1024 * sizeof(FileBlock));

  // 初始化根目录
  inode *root = reinterpret_cast<inode *>(&fileBlocks[0]);
  root->mode = 0x8000;

  // 设置位图
  BITMAP[0] |= 0x8000000000000000;
  // 写入磁盘

  file.seekp(0, std::ios::beg);

  file.write(reinterpret_cast<char *>(fileBlocks), 1024 * sizeof(FileBlock));
  file.write(reinterpret_cast<char *>(BITMAP), 16 * sizeof(uint64_t));
  // 刷新缓冲区
  return 0;
}

// addr为地址数组，长度为blockNum
int getFreeBlock(uint16_t *addr, uint16_t blockNum) {

  // 读取位图
  file.seekg(1024 * sizeof(FileBlock), std::ios::beg);
  file.read(reinterpret_cast<char *>(BITMAP), 16 * sizeof(uint64_t));
  // 遍历位图,找到空闲块并设置位图
  for (int i = 0; i < 16; i++) {
    if (BITMAP[i] == 0xffffffffffffffff) {
      continue;
    }
    for (int j = 0; blockNum && j < 64; j++) {
      if ((BITMAP[i] & (0x8000000000000000 >> j)) == 0) {
        BITMAP[i] |= (0x8000000000000000 >> j);
        addr[blockNum - 1] = i * 64 + j;
        blockNum--;
      }
    }
  }
  // 写入位图
  if (blockNum == 0) {
    file.seekp(1024 * sizeof(FileBlock), std::ios::beg);
    file.write(reinterpret_cast<char *>(BITMAP), 16 * sizeof(uint64_t));
    file.close();
    file.open("disk.dat", std::ios::in | std::ios::out | std::ios::binary);
    return 0;
  }
  // 空间不足
  return -1;
}

int deleteBlock(uint16_t *addr, uint16_t blockNum) {

  // 读取位图
  file.seekg(1024 * sizeof(FileBlock), std::ios::beg);
  file.read(reinterpret_cast<char *>(BITMAP), 16 * sizeof(uint64_t));

  // 遍历位图,找到非空闲块并设置位图
  for (int i = 0; i < blockNum; i++) {
    BITMAP[addr[i] / 64] &= ~(0x8000000000000000 >> (addr[i] % 64));
  }
  // 写入位图
  file.seekp(1024 * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(BITMAP), 16 * sizeof(uint64_t));
  file.close();
  file.open("disk.dat", std::ios::in | std::ios::out | std::ios::binary);
  return 0;
}

int deleteInode(uint16_t dir_addr, uint16_t addr) {
  if (addr == 0) { // 根目录不可删除
    return -1;
  }
  // 读取父目录
  file.seekg(dir_addr * sizeof(FileBlock), std::ios::beg);
  inode dir;
  file.read(reinterpret_cast<char *>(&dir), sizeof(FileBlock));

  // 查找父目录内容，删除目录项
  DirBlock dir_block;
  file.seekg(dir.addr[(dir.size - 1) / 4] * sizeof(FileBlock),
             std::ios::beg); // 最后一个目录块
  file.read(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));

  dir_item temp = dir_block.data[dir.size % 4 - 1]; // 最后一个目录项

  for (int i = 0; i < dir.size % 4; i++) { // 最后一个目录块
    if (dir_block.data[i].addr == addr) {
      dir_block.data[i] = temp; // 覆盖
      dir.size--;
      // 写入父目录块文件
      file.seekp(dir.addr[(dir.size - 1) / 4] * sizeof(FileBlock),
                 std::ios::beg);
      file.write(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));
      break;
    }
  }

  for (int i = 0; i < (dir.size - 1) / 4; i++) { // 其他目录块
    // 读取目录块
    file.seekg(dir.addr[i] * sizeof(FileBlock), std::ios::beg);
    file.read(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));

    for (int j = 0; j < 4; j++) { // 遍历目录项
      if (dir_block.data[j].addr == addr) {
        dir_block.data[j] = temp; // 覆盖
        dir.size--;
        // 写入父目录块文件
        file.seekp(dir.addr[i] * sizeof(FileBlock), std::ios::beg);
        file.write(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));
        break;
      }
    }
  }
  // 写入父目录inode
  file.seekp(dir_addr * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(&dir), sizeof(FileBlock));

  // 文件删除，读入文件inode
  file.seekg(addr * sizeof(FileBlock), std::ios::beg);
  inode file_node;
  file.read(reinterpret_cast<char *>(&file_node), sizeof(FileBlock));

  if (file_node.size)
  // 将文件块装入addr_list
  {
    uint16_t addr_list[(file_node.size - 1) / 40 + 1 + 1];
    memcpy(addr_list, file_node.addr,
           sizeof(uint16_t) * ((file_node.size - 1) / 40 + 1));
    addr_list[(file_node.size - 1) / 40 + 1] = addr;
    // 删除文件块
    deleteBlock(addr_list, (file_node.size - 1) / 40 + 1 + 1);
  } else {
    deleteBlock(&addr, 1);
  }

  return 0;
}

int createFile(uint16_t dir_addr, const char *name, uint16_t *file_addr,
               char *buffer, int size) {
  // dir_addr为父目录地址，name为文件名，file_addr为文件地址，buffer为文件内容，size为文件大小

  // 申请空闲块
  uint16_t addr[(size - 1) / 40 + 1 + 1];
  getFreeBlock(addr, (size - 1) / 40 + 1 + 1);
  *file_addr = addr[0]; // 文件inode地址

  // 写入文件inode块
  inode file_node;
  file_node.mode = 0x0000;
  file_node.size = size;
  memcpy(file_node.addr, addr + 1, sizeof(uint16_t) * ((size - 1) / 40 + 1));
  file.seekp(addr[0] * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(&file_node), sizeof(FileBlock));

  for (int i = 1; i < (size - 1) / 40 + 1 + 1; i++) { // 写入文件内容
    file.seekp(addr[i] * sizeof(FileBlock), std::ios::beg);
    file.write(buffer + (i - 1) * 40, sizeof(FileBlock));
  }

  // 读取父目录，写入目录项
  file.seekg(dir_addr * sizeof(FileBlock), std::ios::beg);
  inode dir;
  file.read(reinterpret_cast<char *>(&dir), sizeof(FileBlock));
  DirBlock dir_block;
  // 写入目录项
  if (dir.size % 4 == 0) {
    getFreeBlock(&dir.addr[dir.size / 4], 1);
  } else {
    file.seekg(dir.addr[dir.size / 4] * sizeof(FileBlock), std::ios::beg);
    file.read(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));
  }
  dir_block.data[dir.size % 4].addr = addr[0];
  memcpy(dir_block.data[dir.size % 4].name, name, 8);
  file.seekp(dir.addr[dir.size / 4] * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));
  // 写入父目录inode
  dir.size++;
  file.seekp(dir_addr * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(&dir), sizeof(FileBlock));
  file.close();
  file.open("disk.dat", std::ios::in | std::ios::out | std::ios::binary);
  return 0;
}

int createDir(uint16_t dir_addr, char *name, uint16_t *child_dir_addr) {

  // 申请空闲块
  uint16_t addr;
  getFreeBlock(&addr, 1);
  *child_dir_addr = addr; // 子目录inode地址

  // 写入子目录inode块
  inode dir_node;
  dir_node.mode = 0x8000;
  dir_node.size = 0;
  file.seekp(addr * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(&dir_node), sizeof(FileBlock));

  // 读取父目录，写入目录项
  file.seekg(dir_addr * sizeof(FileBlock), std::ios::beg);
  inode dir;
  file.read(reinterpret_cast<char *>(&dir), sizeof(FileBlock));
  DirBlock dir_block;
  // 写入目录项
  if (dir.size % 4 == 0) {
    getFreeBlock(&dir.addr[dir.size / 4], 1);
  } else {
    file.seekg(dir.addr[dir.size / 4] * sizeof(FileBlock), std::ios::beg);
    file.read(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));
  }
  dir_block.data[dir.size % 4].addr = addr;
  memcpy(dir_block.data[dir.size % 4].name, name, 8);
  file.seekp(dir.addr[dir.size / 4] * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));

  dir.size++;
  file.seekp(dir_addr * sizeof(FileBlock), std::ios::beg);
  file.write(reinterpret_cast<char *>(&dir), sizeof(FileBlock));
  file.close();
  file.open("disk.dat", std::ios::in | std::ios::out | std::ios::binary);
  return 0;
}

int loadFile(uint16_t addr, char *buffer) {
  // 读取文件inode
  file.seekg(addr * sizeof(FileBlock), std::ios::beg);
  inode file_node;
  file.read(reinterpret_cast<char *>(&file_node), sizeof(FileBlock));

  for (int i = 0; i < (file_node.size - 1) / 40 + 1; i++) { // 读取文件内容
    file.seekg(file_node.addr[i] * sizeof(FileBlock), std::ios::beg);
    file.read(buffer + i * 40, sizeof(FileBlock));
  }

  return 0;
}

int readDir(uint16_t addr, FCB *buffer, uint16_t *size) {
  // 读取目录inode
  file.seekg(addr * sizeof(FileBlock), std::ios::beg);
  inode dir;
  file.read(reinterpret_cast<char *>(&dir), sizeof(FileBlock));
  *size = dir.size;
  uint8_t pointer = 0;

  // 查找目录内容
  DirBlock dir_block;
  file.seekg(dir.addr[(dir.size - 1) / 4] * sizeof(FileBlock),
             std::ios::beg); // 最后一个目录块
  file.read(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));

  inode child_inode;
  for (int i = 0; i < dir.size % 4; i++) { // 最后一个目录块
    buffer[pointer].addr = dir_block.data[i].addr;
    memcpy(buffer[pointer].name, dir_block.data[i].name, 8);
    file.seekg(dir_block.data[i].addr * sizeof(FileBlock), std::ios::beg);
    file.read(reinterpret_cast<char *>(&child_inode), sizeof(FileBlock));
    buffer[pointer].mode = child_inode.mode;
    buffer[pointer].size = child_inode.size;
    pointer++;
  }

  for (int i = 0; i < (dir.size - 1) / 4; i++) { // 其他目录块
    // 读取目录块
    file.seekg(dir.addr[i] * sizeof(FileBlock), std::ios::beg);
    file.read(reinterpret_cast<char *>(&dir_block), sizeof(DirBlock));

    for (int j = 0; j < 4; j++) { // 遍历目录项
      buffer[pointer].addr = dir_block.data[j].addr;
      memcpy(buffer[pointer].name, dir_block.data[j].name, 8);
      file.seekg(dir_block.data[j].addr * sizeof(FileBlock), std::ios::beg);
      file.read(reinterpret_cast<char *>(&child_inode), sizeof(FileBlock));
      buffer[pointer].mode = child_inode.mode;
      buffer[pointer].size = child_inode.size;
      pointer++;
    }
  }
  return 0;
}
