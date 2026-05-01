// ---------------------------
// AsyncTaskMgr.h
// ---------------------------
#include <functional>
#include <queue>
#include <mutex>
#include <thread>

// 补偿任务类型（你代码里的 COMPENSATE_REDIS_DEL）
enum CompensateType {
    COMPENSATE_REDIS_DEL,  // 删除Redis缓存
    COMPENSATE_DB_UPDATE,  // 数据库重试
};

// 异步任务管理器（单例）
class AsyncTaskMgr {
public:
    static AsyncTaskMgr* Instance();

    // 提交补偿任务 → 就是你用的那个函数！
    void async_add_compensate_task(CompensateType type, uint64_t player_id);

    // 后台线程执行
    void run();

private:
    AsyncTaskMgr();
    std::queue<std::pair<CompensateType, uint64_t>> m_task_queue;
    std::mutex m_mutex;
    std::thread m_thread;
    bool m_running;
};

// 全局快捷调用宏（和你代码完全一致）
#define async_add_compensate_task(type, pid) \
    AsyncTaskMgr::Instance()->async_add_compensate_task(type, pid)