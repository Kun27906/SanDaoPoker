// SanDaoPoker 框架入口(第1天空工程骨架,保证可编译)
#include <iostream>

int main() {
#ifdef WITH_SFML
    std::cout << "SanDaoPoker v0.1 (SFML 模式) - 框架编译成功!" << std::endl;
#else
    std::cout << "SanDaoPoker v0.1 框架编译成功! (纯C++骨架,渲染模块第2天接入)"
              << std::endl;
#endif
    return 0;
}
