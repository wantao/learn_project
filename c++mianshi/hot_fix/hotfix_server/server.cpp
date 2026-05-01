#include <iostream>
#include <atomic>
#include <dlfcn.h>
#include <thread>
#include <vector>
#include <unistd.h>
#include "logic_api.h"

// 全局原子函数指针
std::atomic<CalcDamageFunc> g_calc_damage{nullptr};

// 保存当前库句柄，用于延迟卸载
static void* g_lib_handle = nullptr;

// 加载指定动态库
bool load_logic_lib(const char* libPath)
{
    // RTLD_NOW 立即解析符号，RTLD_GLOBAL 全局符号可见
    void* handle = dlopen(libPath, RTLD_NOW | RTLD_GLOBAL);
    if (!handle)
    {
        std::cerr << "dlopen fail: " << dlerror() << std::endl;
        return false;
    }

    // 获取导出函数
    CalcDamageFunc func = (CalcDamageFunc)dlsym(handle, "calculate_damage");
    if (!func)
    {
        std::cerr << "dlsym fail: " << dlerror() << std::endl;
        dlclose(handle);
        return false;
    }

    // 原子替换：release 保证库初始化完再暴露指针
    CalcDamageFunc old = g_calc_damage.exchange(func, std::memory_order_release);

    // 旧句柄延迟一会再关，给正在执行的请求收尾
    if (g_lib_handle)
    {
        std::thread([](){
            sleep(2);  // 等待2秒，让存量请求跑完
            dlclose(g_lib_handle);
        }).detach();
    }

    g_lib_handle = handle;
    std::cout << "加载逻辑库成功: " << libPath << std::endl;
    return true;
}

// 模拟战斗业务线程
void battle_worker(int id)
{
    while (true)
    {
        // acquire 读取：保证拿到最新已初始化的函数指针
        CalcDamageFunc func = g_calc_damage.load(std::memory_order_acquire);
        if (!func)
        {
            sleep(1);
            continue;
        }

        int hp = 1000;
        int atk = 200;
        int def = 50;
        int remain = func(hp, atk, def);

        std::cout << "线程" << id 
                  << " 剩余血量: " << remain << std::endl;
        sleep(1);
    }
}

// 控制台热更交互
void console_loop()
{
    std::string cmd;
    while (std::cin >> cmd)
    {
        if (cmd == "hotfix")
        {
            load_logic_lib("./libbattle_new.so");
        }
        else if (cmd == "rollback")
        {
            load_logic_lib("./libbattle_old.so");
        }
        else if (cmd == "exit")
        {
            std::cout << "退出服务器" << std::endl;
            _exit(0);
        }
        else
        {
            std::cout << "可用命令：hotfix | rollback | exit" << std::endl;
        }
    }
}

int main()
{
    // 初始加载旧逻辑库
    load_logic_lib("./libbattle_old.so");

    // 启动8个战斗工作线程
    std::vector<std::thread> workers;
    for (int i = 0; i < 8; ++i)
    {
        workers.emplace_back(battle_worker, i);
    }

    // 控制台监听热更命令
    console_loop();

    for (auto& t : workers) t.join();
    return 0;
}