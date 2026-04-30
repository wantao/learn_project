#include <event2/event.h>
#include <event2/bufferevent.h>
#include <sys/socket.h>

// 客户端消息回调（游戏协议处理）
void read_cb(struct bufferevent *bev, void *ctx) {
    char buf[1024];
    bufferevent_read(bev, buf, sizeof(buf));
    printf("收到游戏客户端消息：%s\n", buf);
}

// 监听回调
void accept_cb(evutil_socket_t fd, short event, void *arg) {
    struct event_base *base = (struct event_base*)arg;
    evutil_socket_t cfd = accept(fd, nullptr, nullptr);
    // 创建缓冲事件管理客户端
    struct bufferevent *bev = bufferevent_socket_new(base, cfd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, read_cb, nullptr, nullptr, nullptr);
    bufferevent_enable(bev, EV_READ);
}

int main() {
    // 1. 创建事件循环
    struct event_base *base = event_base_new();
    // 2. 创建监听socket
    evutil_socket_t lfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr{AF_INET, htons(8888), INADDR_ANY};
    bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(lfd, 128);
    // 3. 注册监听事件
    struct event *ev = event_new(base, lfd, EV_READ|EV_PERSIST, accept_cb, base);
    event_add(ev, nullptr);
    // 4. 启动循环
    event_base_dispatch(base);
    return 0;
}
// 编译：g++ -o server server.cpp -levent
// 测试：telnet 127.0.0.1 8888 发送消息，服务器可接收