#include <atomic>
#include <dlfcn.h>
#include <iostream>

// 战斗函数类型定义（关键：统一签名）
using BattleFunc = int (*)(int player_hp, int enemy_atk);

// 原子函数指针：多线程安全，热更核心
std::atomic<BattleFunc> g_battle_func{nullptr};

// 初始加载旧战斗库
bool load_old_battle() {
    void* handle = dlopen("./libbattle_old.so", RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        std::cerr << dlerror() << std::endl;
        return false;
    }
    // 获取函数地址并原子赋值
    BattleFunc func = (BattleFunc)dlsym(handle, "calculate_damage");
    g_battle_func.store(func, std::memory_order_release);
    return true;
}

// 热更新：加载新战斗库
bool hotfix_new_battle() {
    void* handle = dlopen("./libbattle_new.so", RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        std::cerr << dlerror() << std::endl;
        return false;
    }
    BattleFunc new_func = (BattleFunc)dlsym(handle, "calculate_damage");
    // 原子替换：所有线程瞬间切换到新逻辑
    g_battle_func.store(new_func, std::memory_order_release);
    std::cout << "战斗逻辑热更成功！" << std::endl;
    return true;
}

// 游戏主循环：统一通过函数指针调用战斗逻辑
void game_loop() {
    int hp = 100, atk = 20;
    // 内存屏障保证读取最新指针
    BattleFunc func = g_battle_func.load(std::memory_order_acquire);
    int damage = func(hp, atk);
    std::cout << "最终伤害：" << damage << std::endl;
}

int main() {
    load_old_battle();
    game_loop();   // 使用旧逻辑
    
    hotfix_new_battle(); // 热更，不停止服务
    game_loop();   // 自动使用新逻辑
    return 0;
}