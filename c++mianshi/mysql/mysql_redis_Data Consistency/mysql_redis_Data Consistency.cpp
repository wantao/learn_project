#include <string>
#include <mutex>
#include <unordered_map>

// 玩家锁（防止同一个玩家并发改钻石，最关键！）
std::unordered_map<uint64_t, std::mutex> g_player_mutex;
std::mutex g_map_mutex;

// Redis 操作（伪代码）
bool redis_del(const std::string& key);
bool redis_get(const std::string& key, uint32_t& val);

// MySQL 操作（伪代码）
bool update_mysql_diamond(uint64_t pid, uint32_t new_diamond);
bool get_mysql_diamond(uint64_t pid, uint32_t& diamond);

// 缓存key
inline std::string get_diamond_key(uint64_t pid) {
    return "player:diamond:" + std::to_string(pid);
}

// 充值成功后，增加钻石
bool add_player_diamond(uint64_t player_id, uint32_t add_num) {
    // ====================== 1. 玩家加锁（防并发覆盖）======================
    std::lock_guard<std::mutex> lock(g_player_mutex[player_id]);

    // ====================== 2. 查最新钻石（加锁后查）======================
    uint32_t old_diamond = 0;
    if (!get_mysql_diamond(player_id, old_diamond)) {
        return false;
    }

    uint32_t new_diamond = old_diamond + add_num;

    // ====================== 3. 更新数据库（必须成功）======================
    if (!update_mysql_diamond(player_id, new_diamond)) {
        // DB 失败直接返回，不能继续
        return false;
    }

    // ====================== 4. 删除缓存（核心！带重试）======================
    std::string key = get_diamond_key(player_id);
    bool del_ok = false;
    
    // 重试 3 次，解决瞬时网络抖动
    for (int i = 0; i < 3; ++i) {
        del_ok = redis_del(key);
        if (del_ok) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    // ====================== 5. 删除失败 → 异步补偿任务 ====================
    if (!del_ok) {
        // 丢到异步任务队列，后台定时重试删缓存
        // 最终一定能删成功 → 保证最终一致
        async_add_compensate_task(COMPENSATE_REDIS_DEL, player_id);
    }

    // ====================== 6. 成功返回 ====================
    return true;
}
// 玩家查询钻石时，自动保证缓存一致
uint32_t get_player_diamond(uint64_t player_id) {
    std::string key = get_diamond_key(player_id);
    uint32_t diamond = 0;

    // 1. 读缓存
    if (redis_get(key, diamond)) {
        return diamond;
    }

    // 2. 缓存不存在 → 读数据库
    std::lock_guard<std::mutex> lock(g_player_mutex[player_id]);
    if (redis_get(key, diamond)) { // 二次检查
        return diamond;
    }

    if (!get_mysql_diamond(player_id, diamond)) {
        return 0;
    }

    // 3. 回写缓存（设置过期时间，自动兜底）
    redis_setex(key, diamond, 86400); // 1天过期
    return diamond;
}

// 后台异步线程执行
void compensate_redis_del(uint64_t player_id) {
    std::string key = get_diamond_key(player_id);
    for (int i = 0; i < 10; ++i) {
        if (redis_del(key)) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // 还失败 → 记录日志，人工兜底/定时任务扫表
}