#include <iostream>
#include <thread>
#include <mutex>
#include <unistd.h>
using namespace std;

// 两个玩家锁（模拟游戏玩家对象）
mutex playerA;
mutex playerB;

// 线程1：先锁A，再锁B
void tradeThread1() {
    cout << "线程1：尝试锁住玩家A..." << endl;
    playerA.lock();
    cout << "线程1：已锁住玩家A" << endl;

    sleep(1); // 强制让线程2先锁住B，制造死锁

    cout << "线程1：尝试锁住玩家B..." << endl;
    playerB.lock(); // 这里会永久等待！
    cout << "线程1：交易完成" << endl;

    playerB.unlock();
    playerA.unlock();
}

// 线程2：先锁B，再锁A 【加锁顺序相反 → 死锁】
void tradeThread2() {
    cout << "线程2：尝试锁住玩家B..." << endl;
    playerB.lock();
    cout << "线程2：已锁住玩家B" << endl;

    sleep(1);

    cout << "线程2：尝试锁住玩家A..." << endl;
    playerA.lock(); // 这里会永久等待！
    cout << "线程2：交易完成" << endl;

    playerA.unlock();
    playerB.unlock();
}

int main() {
    cout << "=== 游戏服务器交易系统 启动 ===" << endl;

    thread t1(tradeThread1);
    thread t2(tradeThread2);

    t1.join();
    t2.join();

    cout << "=== 服务器正常结束 ===" << endl; // 永远不会执行！
    return 0;
}