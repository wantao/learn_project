#include <iostream>
#include <memory>
#include <vector>
#include <list>

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

// 模拟游戏角色对象
class GameRole {
public:
    GameRole(int id) : id(id) {}
private:
    int id;
};

int main() {
    // 3. 移动语义优化容器存储
    std::vector<GameObject> vec;
    vec.reserve(2); // 预留空间，避免多次扩容
    vec.emplace_back(); // 原地构造，无拷贝/移动
    vec.push_back(GameObject()); // 移动构造，无拷贝

    // 优化：预分配内存，避免扩容
    std::vector<GameRole> role_vec;
    role_vec.reserve(1000); // 一次性分配1000容量，无扩容
    std::cout << "11vector 容量: " << role_vec.capacity() << "\n";

    // 原地构造，无拷贝
    role_vec.emplace_back(1001);
    role_vec.emplace_back(1002);

    // 清理复用，不重建容器
    role_vec.clear();
    role_vec.shrink_to_fit(); // 释放多余内存

    // list 仅用于频繁插入删除
    std::list<GameRole> role_list;
    role_list.emplace_back(2001);

    std::cout << "vector 容量: " << role_vec.capacity() << "\n";

    return 0;
}