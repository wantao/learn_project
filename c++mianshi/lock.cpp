#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

std::mutex g_mutex;
std::atomic<int> g_atomic_count{0};  // 修复在这里！
int g_normal_count = 0;

// 线程安全：lock_guard
void safe_task() {
    for (int i = 0; i < 10000; ++i) {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_normal_count++;
    }
}

// 无锁原子操作
void atomic_task() {
    for (int i = 0; i < 10000; ++i) {
        g_atomic_count++;
    }
}

int main() {
    std::thread t1(safe_task);
    std::thread t2(safe_task);
    t1.join();
    t2.join();
    std::cout << "锁保护计数: " << g_normal_count << "\n";

    std::thread t3(atomic_task);
    std::thread t4(atomic_task);
    t3.join();
    t4.join();
    std::cout << "原子计数: " << g_atomic_count << "\n";
    return 0;
}