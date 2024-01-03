#include <cstring>
#include <memory_manage.h>

MemoryBlock memory[64];

uint8_t BITMAP;

PageTable page_table[8];

int malloc_memory(uint8_t *index) {

  for (int i = 0; i < 8; i++) { // 找到第一个空闲的大内存块
    if ((BITMAP & (1 << i)) == 0) {
      BITMAP |= (1 << i);
      *index = i; // 返回大内存块号
      return 0;
    }
  }
  return -1; // 内存已满
}

int free_memory(uint8_t index) {
  BITMAP &= ~(1 << index);
  return 0;
}

int request_page(uint8_t index, uint8_t page_id, MemoryBlock *data) {
  if (index >= 8 || page_id >= page_table[index].size) { // 越界
    return -1;
  }

  if (page_table[index].is_valid[page_id]) { // 页面在内存中
    *data = memory[page_table[index].page_addr[page_id]];
    return 0;
  } else { // 页面不在内存中,从磁盘中读取
    uint8_t page_index = 0;
    if (page_table[index].is_full) { // 内存已满
      // 选一页换出，找到最先进入内存的页
      for (int i = 0; i < 18; i++) {
        if (page_table[index].is_valid[i] &&
            page_table[index].page_addr[i] ==
                index * 8 + page_table[index].point) {
          page_index = i;
          break;
        }
      }
      // 是否被修改
      if (page_table[index].is_dirty[page_index]) {
        // 写回磁盘
        /// 待补充
      }
      page_table[index].is_valid[page_index] = false; // 不在内存中
    } else {                                          // 内存未满
      if (page_table[index].point == 7) {
        page_table[index].is_full = true;
      }
    }
    // 从磁盘中读取
    MemoryBlock tmp;
    /// 待补充
    // 读入内存
    memory[index * 8 + page_table[index].point] = tmp;
    *data = tmp;
    // 更新页表
    page_table[index].is_valid[page_id] = true;  // 在内存中
    page_table[index].is_dirty[page_id] = false; // 未修改
    page_table[index].page_addr[page_id] =
        index * 8 + page_table[index].point; // 块号
    page_table[index].point = (page_table[index].point + 1) %
                              8; // 内存满时最先进入内存的页，未满时下一个空闲块

    return 0;
  }
}

int load_file(uint8_t index, uint8_t size, uint16_t *inode_addr) {
  if (index >= 8 || size > 18) { // 错误
    return -1;
  }
  page_table[index].size = size;
  page_table[index].is_full = false;
  page_table[index].point = 0;
  for (int i = 0; i < size; i++) {
    page_table[index].inode_addr[i] = inode_addr[i];
    page_table[index].is_valid[i] = false;
    page_table[index].is_dirty[i] = false;
  }
  return 0;
}