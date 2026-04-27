#include <iostream>
#include <memory>
#include <vector>

// 模拟游戏对象
class GameObject {
public:
    GameObject() { std::cout << "创建GameObject\n"; }
    ~GameObject() { std::cout << "销毁GameObject\n"; }
    // 禁用拷贝，只允许移动
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;
    GameObject(GameObject&&) noexcept = default;
    GameObject& operator=(GameObject&&) noexcept = default;
};

int main() {
    // 1. unique_ptr 独占所有权
    // 修复：C++11 兼容写法
    std::unique_ptr<GameObject> obj1(new GameObject());
    std::unique_ptr<GameObject> obj2 = std::move(obj1); // 所有权转移

    return 0;
}