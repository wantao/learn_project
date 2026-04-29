#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <functional>
#include <iostream>

using namespace std;

// ------------------------------
// 线程池（IO / 逻辑通用）
// ------------------------------
class ThreadPool {
public:
    using Task = std::function<void()>;

    ThreadPool(int thread_num) {
        for (int i = 0; i < thread_num; ++i) {
            threads_.emplace_back(&ThreadPool::work_loop, this);
        }
    }

    void enqueue(Task task) {
        std::lock_guard<std::mutex> lock(mtx_);
        tasks_.push_back(std::move(task));
    }

private:
    void work_loop() {
        while (true) {
            Task task;
            {
                std::lock_guard<std::mutex> lock(mtx_);
                if (!tasks_.empty()) {
                    task = std::move(tasks_.front());
                    tasks_.pop_front();
                }
            }

            if (task) {
                task();
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    std::vector<std::thread> threads_;
    std::deque<Task> tasks_;
    std::mutex mtx_;
};

// ------------------------------
// 全局变量
// ------------------------------
int listen_fd;                  // 监听socket
int epfd;                       // epoll句柄
ThreadPool* io_thread_pool;      // IO线程池
ThreadPool* logic_thread_pool;   // 逻辑线程

// ------------------------------
// 端口号：8888
// ------------------------------
const int PORT = 8888;

// ------------------------------
// 初始化监听socket（完整实现）
// ------------------------------
bool init_listen() {
    // 1. 创建socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) return false;

    // 端口复用
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    // 2. 绑定地址与端口 【关键：这里绑定 8888】
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);  // <-- 端口 8888

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind fail");
        return false;
    }

    // 3. 监听
    if (listen(listen_fd, 128) < 0) {
        perror("listen fail");
        return false;
    }

    cout << "server listen on port " << PORT << endl;
    return true;
}

// ------------------------------
// 接受新连接
// ------------------------------
void accept_new_conn() {
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    while (true) {
        int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &len);
        if (client_fd < 0) break;

        // 设置非阻塞
        int flags = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

        // 加入epoll
        epoll_event event;
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = client_fd;
        epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &event);

        cout << "new client: " << client_fd << endl;
    }
}

// ------------------------------
// 读取客户端数据
// ------------------------------
void do_read(int fd) {
    char buffer[4096];
    while (true) {
        int n = recv(fd, buffer, sizeof(buffer), 0);
        if (n <= 0) break;

        cout << "recv from " << fd << ": " << string(buffer, n) << endl;

        // 扔给逻辑线程处理
        logic_thread_pool->enqueue([=]() {
            send(fd, "ok\n", 3, 0);
        });
    }

    close(fd);
}

// ------------------------------
// 主Reactor
// ------------------------------
void net_reactor_loop() {
    epoll_event events[512];
    while (true) {
        int n = epoll_wait(epfd, events, 512, -1);
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;

            if (fd == listen_fd) {
                io_thread_pool->enqueue(std::bind(accept_new_conn));
            } else if (events[i].events & EPOLLIN) {
                io_thread_pool->enqueue([=]() { do_read(fd); });
            }
        }
    }
}

// ------------------------------
// main
// ------------------------------
int main() {
    // 初始化监听
    if (!init_listen()) {
        return -1;
    }

    // 创建epoll
    epfd = epoll_create1(0);
    epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &event);

    // 线程池
    io_thread_pool = new ThreadPool(4);
    logic_thread_pool = new ThreadPool(8);

    // 启动网络主循环
    net_reactor_loop();

    return 0;
}