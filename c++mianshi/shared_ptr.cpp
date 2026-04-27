#include <iostream>
#include <memory>
#include <vector>

void test_wild_pointer() {
    int* p = new int(10);
    delete p;
    // p = nullptr; // 不写这行就是野指针
    // std::cout << *p << std::endl; // 未定义行为，游戏崩溃
}

// RAII 管理自定义资源
class GameTexture {
public:
    GameTexture() { std::cout << "加载纹理\n"; }
    ~GameTexture() { std::cout << "释放纹理\n"; }
};

// 智能指针自动管理，无需手动释放
void use_texture() {
    std::shared_ptr<GameTexture> tex = std::make_shared<GameTexture>();
    // 函数退出自动释放纹理，无泄漏
}


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

    // 2. shared_ptr 共享引用
    std::shared_ptr<GameObject> s_ptr1 = std::make_shared<GameObject>();
    std::shared_ptr<GameObject> s_ptr2 = s_ptr1;
    std::cout << "shared_ptr 引用计数: " << s_ptr1.use_count() << "\n";

    test_wild_pointer();
    use_texture();

    return 0;
}