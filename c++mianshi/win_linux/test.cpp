#include <iostream>
#include <thread>
// 跨平台网络头文件
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <unistd.h>
#endif

// 跨平台线程函数
void game_thread_func() {
    std::cout << "游戏逻辑线程运行中" << std::endl;
}

int main() {
    // 1. 跨平台网络初始化（Windows必须执行，Linux空实现）
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    std::cout << "Windows 网络初始化完成" << std::endl;
#else
    std::cout << "Linux 无需初始化网络" << std::endl;
#endif

    // 2. 跨平台线程（C++11标准，无差异）
    std::thread t(game_thread_func);
    t.join();

    // 3. 跨平台清理
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}