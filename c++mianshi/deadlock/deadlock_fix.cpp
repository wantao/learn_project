#include <iostream>
#include <thread>
#include <mutex>
#include <unistd.h>
using namespace std;

mutex playerA;
mutex playerB;

// 统一锁顺序：先锁A，再锁B
void tradeSafe(int id) {
    // 固定顺序：永远先A后B
    lock_guard<mutex> lockA(playerA);
    lock_guard<mutex> lockB(playerB);

    cout << "线程" << id << "：交易完成（无死锁）" << endl;
}

int main() {
    cout << "=== 修复后：无死锁版本 ===" << endl;

    thread t1(tradeSafe, 1);
    thread t2(tradeSafe, 2);

    t1.join();
    t2.join();

    cout << "=== 服务器正常结束 ===" << endl;
    return 0;
}