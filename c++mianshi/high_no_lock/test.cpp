#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

// 游戏高并发无锁队列（简化版）
template<typename T>
class LockFreeQueue {
private:
    struct Node {
        T data;
        std::atomic<Node*> next;
        Node(T val) : data(val), next(nullptr) {}
    };
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
public:
    LockFreeQueue() {
        Node* dummy = new Node(T());
        head = dummy;
        tail = dummy;
    }

    // 无锁入队
    void push(T val) {
        Node* new_node = new Node(val);
        Node* old_tail = tail.exchange(new_node);
        old_tail->next = new_node;
    }

    // 无锁出队
    bool pop(T& val) {
        Node* old_head = head.load();
        Node* next = old_head->next.load();
        if (next == nullptr) return false;
        val = next->data;
        head = next;
        delete old_head;
        return true;
    }
};

// 测试：10个线程并发入队，1个线程出队
void test_high_concurrent() {
    LockFreeQueue<int> q;
    std::vector<std::thread> threads;

    // 10个并发生产者（模拟10个游戏客户端发包）
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 1000; ++j) {
                q.push(j);
            }
        });
    }

    // 消费者（模拟游戏逻辑线程）
    int count = 0;
    while (count < 10 * 1000) {
        int val;
        if (q.pop(val)) count++;
    }

    for (auto& t : threads) t.join();
    std::cout << "高并发测试完成，总处理消息数：" << count << std::endl;
}

int main() {
    test_high_concurrent();
    return 0;
}