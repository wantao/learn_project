#include <iostream>
#include <thread>
#include <unistd.h>
using namespace std;

// 游戏玩家对象（持续new，不delete → 内存泄漏）
class Player {
    char data[4096];  // 每个对象占 4KB 内存
public:
    Player() { }
};

// ===================== 泄漏函数：会被采样抓到 =====================
// 泄漏点：循环创建玩家，绝不释放
void createPlayerLeak() {
    // 持续分配内存，不释放
    Player* p = new Player();
}

// 游戏服务端：持续运行
void serverLoop() {
    while (true) {
        createPlayerLeak();  // 一直调用泄漏函数
        sleep(1);            // 1秒一次，方便采样
    }
}

int main() {
    cout << "=== 游戏服务端启动 (PID: " << getpid() << ") ===" << endl;
    serverLoop();  // 持续运行
    return 0;
}