#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <cstdio>
#include <cstring>

// 崩溃日志文件名
const char* CRASH_LOG_FILE = "./crash_reliable.log";

// 【重点】信号处理函数
// 注意：这里只能使用异步信号安全的函数（不能用new/malloc/printf）
void crashHandler(int sig) {
    // 栈上分配缓冲区（绝对安全，不使用堆）
    char buffer[2048];

    // 拼接崩溃信息
    snprintf(buffer, sizeof(buffer),
             "[CRASH] 信号ID: %d | 信号名: %s | 时间戳: %lu\n"
             "进程已崩溃，此日志保证落盘成功！\n"
             "-----------------------------------------\n",
             sig, strsignal(sig), (unsigned long)time(NULL));

    // 【裸write】直接系统调用，无缓冲、必落盘
    // 打开文件：只写|创建|追加
    int fd = open(CRASH_LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd >= 0) {
        // 写入数据
        write(fd, buffer, strlen(buffer));
        // 强制刷到磁盘（关键！）
        fdatasync(fd);
        close(fd);
    }

    // 恢复默认信号行为，生成core dump
    signal(sig, SIG_DFL);
    // 重新抛出信号，让系统生成core文件
    raise(sig);
}

// 注册崩溃信号
void registerCrashHandler() {
    // 常见崩溃信号全部捕获
    struct sigaction sa{};
    sa.sa_handler = crashHandler;
    // 阻塞其他信号，防止处理时被打断
    sigfillset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    // 段错误（空指针、越界）
    sigaction(SIGSEGV, &sa, NULL);
    // 断言失败
    sigaction(SIGABRT, &sa, NULL);
    // 总线错误
    sigaction(SIGBUS, &sa, NULL);
    // 浮点异常
    sigaction(SIGFPE, &sa, NULL);
    // 非法指令
    sigaction(SIGILL, &sa, NULL);
}

// 测试函数：主动触发崩溃
void makeCrash() {
    // 空指针解引用 → 触发SIGSEGV
    int* p = nullptr;
    *p = 666;
}

int main() {
    // 注册崩溃处理
    registerCrashHandler();
    std::cout << "程序启动，即将触发崩溃...\n";

    // 触发崩溃
    makeCrash();

    return 0;
}