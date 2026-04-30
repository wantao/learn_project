#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <string>

// 游戏玩家消息结构体
struct PlayerMsg
{
    int playerId;
    std::string cmd;   // 指令：登录、领奖、聊天
};

// 全局游戏共享资源
// 1. 玩家基础数据
int g_playerGold = 1000;
std::mutex g_dataMtx;          // 玩家数据锁

// 2. 玩家消息队列 + 条件变量
std::queue<PlayerMsg> g_msgQueue;
std::mutex g_queueMtx;
std::condition_variable g_cv; // 条件变量
bool g_serverRun = true;

// ======================
// 1. 玩家数据读写：只用 lock_guard
// ======================
// 增加玩家金币
void addPlayerGold(int addVal)
{
    // lock_guard：构造加锁，离开作用域自动解锁
    std::lock_guard<std::mutex> lock(g_dataMtx);
    g_playerGold += addVal;
    std::cout << "当前玩家金币: " << g_playerGold << std::endl;
}

// 查询玩家金币
int getPlayerGold()
{
    std::lock_guard<std::mutex> lock(g_dataMtx);
    return g_playerGold;
}

// ======================
// 2. 消息队列消费者线程：必须用 unique_lock + 条件变量
// ======================
void msgConsumeThread()
{
    while (g_serverRun)
    {
        // unique_lock 支持 解锁、wait、延迟加锁，条件变量必备
        std::unique_lock<std::mutex> lock(g_queueMtx);

        // 等待队列有消息，没消息就阻塞休眠，释放锁让生产者能写入
        g_cv.wait(lock, [](){ 
            return !g_msgQueue.empty() || !g_serverRun; 
        });

        // 服务停止直接退出
        if (!g_serverRun) break;

        // 取出队首消息
        PlayerMsg msg = g_msgQueue.front();
        g_msgQueue.pop();

        // 手动解锁，处理业务逻辑时不阻塞生产者
        lock.unlock();

        // 模拟处理玩家消息
        std::cout << "处理玩家[" << msg.playerId << "] 指令: " << msg.cmd << std::endl;
    }
}

// ======================
// 3. 消息队列生产者：往队列发玩家消息
// ======================
void pushPlayerMsg(int pid, const std::string& cmd)
{
    std::lock_guard<std::mutex> lock(g_queueMtx);
    g_msgQueue.push({pid, cmd});
    // 唤醒一个等待的消费线程
    g_cv.notify_one();
}

// ======================
// 测试入口
// ======================
int main()
{
    // 启动消息消费线程
    std::thread consumer(msgConsumeThread);

    // 模拟多线程读写玩家数据
    std::thread t1(addPlayerGold, 200);
    std::thread t2(addPlayerGold, 300);

    t1.join();
    t2.join();

    // 模拟推送多条玩家游戏消息
    pushPlayerMsg(10001, "玩家登录");
    pushPlayerMsg(10002, "领取在线奖励");
    pushPlayerMsg(10001, "发送世界聊天");

    // 休眠等待处理完
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 停止服务
    g_serverRun = false;
    g_cv.notify_one();
    consumer.join();

    std::cout << "最终玩家金币: " << getPlayerGold() << std::endl;
    return 0;
}