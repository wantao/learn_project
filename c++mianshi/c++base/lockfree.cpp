#include <atomic>
using namespace std;

template<typename T>
class LockFreeQueue {
private:
    struct Node {
        T data;
        atomic<Node*> next;
        Node() : next(nullptr) {}
        Node(const T& val) : data(val), next(nullptr) {}
    };

    atomic<Node*> head_;
    atomic<Node*> tail_;

public:
    LockFreeQueue() {
        Node* dummy = new Node();
        head_.store(dummy);
        tail_.store(dummy);
    }

    // 无锁入队
    void enqueue(const T& data) {
        Node* new_node = new Node(data);
        Node* old_tail = nullptr;

        while (true) {
            old_tail = tail_.load();
            Node* next = old_tail->next.load();

            if (old_tail == tail_.load()) {
                if (next == nullptr) {
                    // CAS 尝试把尾节点next指向新节点
                    if (old_tail->next.compare_exchange_weak(next, new_node)) {
                        // CAS 移动尾指针
                        tail_.compare_exchange_weak(old_tail, new_node);
                        return;
                    }
                } else {
                    tail_.compare_exchange_weak(old_tail, next);
                }
            }
        }
    }

    // 无锁出队
    bool dequeue(T& out) {
        Node* old_head = nullptr;

        while (true) {
            old_head = head_.load();
            Node* old_tail = tail_.load();
            Node* next = old_head->next.load();

            if (old_head == head_.load()) {
                if (old_head == old_tail) {
                    if (next == nullptr) return false; // 空
                    tail_.compare_exchange_weak(old_tail, next);
                } else {
                    out = next->data;
                    if (head_.compare_exchange_weak(old_head, next)) {
                        return true;
                    }
                }
            }
        }
    }
};

// 战斗消息队列（无锁）
LockFreeQueue<BattleMsg> g_battle_queue;

// 网络线程：入队
void net_thread() {
    BattleMsg msg;
    g_battle_queue.enqueue(msg);
}

// 战斗线程：出队
void battle_thread() {
    BattleMsg msg;
    if (g_battle_queue.dequeue(msg)) {
        process(msg);  // 无锁执行战斗逻辑
    }
}