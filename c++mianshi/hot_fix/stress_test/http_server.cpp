#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <atomic>
#include <dlfcn.h>
#include <fcntl.h>
#include <iostream>

// 战斗函数签名
typedef int (*BattleFunc)(int hp, int atk);

// 原子函数指针（热更核心）
std::atomic<BattleFunc> g_battle_func;

// 旧库句柄
void* g_old_handle = nullptr;

// 加载动态库
bool load_lib(const char* path) {
    void* handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        printf("dlopen err: %s\n", dlerror());
        return false;
    }

    BattleFunc f = (BattleFunc)dlsym(handle, "calc_damage");
    if (!f) {
        printf("dlsym err\n");
        dlclose(handle);
        return false;
    }

    // 原子替换
    g_battle_func.store(f, std::memory_order_release);

    // 延迟卸载旧库
    if (g_old_handle) {
        sleep(1);
        dlclose(g_old_handle);
    }
    g_old_handle = handle;
    return true;
}

// HTTP 处理（修复版）
// 修复后的 HTTP 处理函数（无报错、稳定、兼容 wrk 压测）
void* handle_client(void* p) {
    int fd = (long long)p;
    char buf[1024];
    ssize_t total_read = 0;

    // ✅ 修复1：保留非阻塞，循环读取直到读完 HTTP 请求
    while (1) {
        ssize_t n = read(fd, buf + total_read, sizeof(buf) - total_read - 1);
        if (n > 0) {
            total_read += n;
            // 读到 HTTP 头结束符 \r\n\r\n 就停止
            if (strstr(buf, "\r\n\r\n")) break;
        } 
        // 无数据了 或 读完
        else if (n == 0 || (n < 0 && total_read > 0)) {
            break;
        }
        // 真错误 / 无数据
        else {
            close(fd);
            return nullptr;
        }
    }

    // 没有读到有效数据，直接关闭（不再打印错误日志）
    if (total_read <= 0) {
        close(fd);
        return nullptr;
    }
    buf[total_read] = 0;

    // 调用热更战斗函数
    BattleFunc f = g_battle_func.load(std::memory_order_acquire);
    int res = f ? f(100, 20) : -1;

    // HTTP 响应
    char body[32];
    snprintf(body, sizeof(body), "%d", res);
    char resp[256];
    snprintf(resp, sizeof(resp),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n\r\n"
        "%s",
        strlen(body), body);

    write(fd, resp, strlen(resp));
    close(fd);
    std::cout << "Response sent to client: " << fd << std::endl;
    return nullptr;
}

int main() {
    // 默认加载旧库
    if (!load_lib("./libbattle_old.so")) {
        printf("load lib failed\n");
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    listen(server_fd, 128);

    printf("Server running on 8080...\n");

    while (1) {
        int fd = accept(server_fd, nullptr, nullptr);
        if (fd < 0) continue;
        std::cout << "New client connected: " << fd << std::endl;
        pthread_t t;
        // 创建线程
        if (pthread_create(&t, nullptr, handle_client, (void*)(long long)fd) == 0) {
            std::cout << "pthread_create successful: " << fd << std::endl;
            pthread_detach(t);
        } else {
            std::cout << "pthread_create failed: " << fd << std::endl;
            close(fd);
        }
    }

    close(server_fd);
    return 0;
}