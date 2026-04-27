#include <iostream>
#include <string>
using namespace std;

// 游戏玩家对象（游戏服务端最常见对象）
class Player {
private:
    int playerId;
    string name;
    // 模拟玩家数据占用内存
    char data[4096]; // 每个玩家占 4KB 内存

public:
    Player(int id, const char* n) : playerId(id), name(n) {
        cout << "玩家登录：" << name << " (ID:" << id << ")" << endl;
    }

    ~Player() {
        cout << "玩家下线，释放内存：" << name << endl;
    }
};

// 模拟游戏逻辑：玩家登录 -> 下线
void playerLoginAndLogout() {
    // ====================== 泄漏点 ======================
    // 玩家登录：new 创建对象
    Player* p = new Player(1001, "TestPlayer_001");

    // 游戏逻辑：玩家做了一些操作...
    // 玩家下线：忘记 delete！！！内存泄漏
    // delete p;  // 这里注释掉 = 故意制造泄漏
}

int main() {
    cout << "游戏服务器启动..." << endl;

    // 模拟 10 次玩家登录/下线
    for (int i = 0; i < 10; i++) {
        playerLoginAndLogout();
    }

    cout << "服务器停止" << endl;
    return 0;
}