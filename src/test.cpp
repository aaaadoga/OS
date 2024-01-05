#include <iostream>
#include <cstdint>

int main() {
    uint16_t uintValue = 65535; // 最大的 uint16_t 值
    int intValue = uintValue;   // 隐式转换

    std::cout << "uintValue: " << uintValue << std::endl;
    std::cout << "intValue: " << intValue << std::endl;

    return 0;
}
