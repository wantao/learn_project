// ---------------------------
// AsyncTaskMgr.cpp
// ---------------------------
#include "AsyncTaskMgr.h"
#include "RedisMgr.h"

AsyncTaskMgr* AsyncTaskMgr::Instance() {
    static AsyncTaskMgr s_inst;
    return &s_inst;
}

AsyncTaskMgr::AsyncTaskMgr() {
    m_running = true;
    m_thread = std::thread(&AsyncTaskMgr::run, this);
}

// 提交任务（你代码里调用的就是它）
void AsyncTaskMgr::async_add_compensate_task(CompensateType type, uint64_t player_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_task_queue.push({type, player_id});
}

// 后台线程循环执行
void AsyncTaskMgr::run() {
    while (m_running) {
        std::lock_guard<std::mutex> lock(m_mutex);

        while (!m_task_queue.empty()) {
            auto [type, player_id] = m_task_queue.front();
            m_task_queue.pop();

            if (type == COMPENSATE_REDIS_DEL) {
                // 重试删除Redis缓存
                std::string key = "player:diamond:" + std::to_string(player_id);
                
                // 重试10次，保证最终一致
                for (int i = 0; i < 10; ++i) {
                    if (RedisMgr::Del(key)) {
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}