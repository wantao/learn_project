#include <iostream>
#include <vector>
#include <unistd.h>
using namespace std;

// 模拟全服玩家（百人同屏）
vector<int> g_allPlayers(10000000);

// 【性能瓶颈】每次伤害计算都遍历全服玩家 O(n)
int CalculateDamage(int playerId)
{
    int total = 0;
    // 高耗时：遍历100个玩家
    for (int i = 0; i < g_allPlayers.size(); i++) {
        total += i;
    }
    return total;
}

// 游戏主循环（每帧执行）
void GameFrame()
{
    // 模拟100个玩家同时释放技能
    for (int i = 0; i < 10000000; i++) {
        CalculateDamage(i);
    }
}

int main()
{
    cout << "=== MMO游戏服务器启动 (PID: " << getpid() << ") ===" << endl;
    while (1) {
        GameFrame();  // 高频执行
        usleep(30000); // 约30帧
        //usleep(3); // 约30帧
    }
    return 0;
}