#include <iostream>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <thread>
#include <string>
#include <atomic>

// 数据库行数据结构
struct BagItem {
    uint32_t item_id;    // 物品ID
    uint32_t count;      // 物品持有数量
};

// 全局统计结果（线程安全）
std::unordered_map<uint32_t, uint64_t> g_item_total;
std::mutex g_map_mutex;       // 哈希表互斥锁
std::atomic<int> g_finished_shards(0);  // 已完成的分表数
const int TOTAL_SHARDS = 128;
// 最大并发线程数（控制数据库压力，推荐16~32）
const int MAX_THREADS = 32;

// 数据库连接句柄（伪代码）
using DBConn = void*;

/**
 * @brief 扫描单个分表，统计物品数量并聚合到全局结果
 * @param shard_id 分表编号 0~127
 */
void stat_shard(int shard_id) {
    // 1. 获取从库只读连接（关键：不连主库）
    DBConn conn = get_slave_db_connection();
    if (!conn) {
        printf("分表%d 数据库连接失败\n", shard_id);
        return;
    }

    // 2. 构造分表名 + 轻量级查询语句（只查需要的字段，无where条件）
    char table_name[64];
    snprintf(table_name, sizeof(table_name), "player_bag_%d", shard_id);
    
    // 优化：流式查询/游标查询，避免一次性加载全表数据到内存
    std::string sql = "SELECT item_id, `count` FROM " + std::string(table_name);
    
    // 3. 执行查询（流式读取，低内存占用）
    DBResult result = db_query_stream(conn, sql.c_str());
    
    // 4. 本地累加（减少锁竞争：先本地统计，最后合并）
    std::unordered_map<uint32_t, uint64_t> local_map;
    
    while (db_fetch_row(result, BagItem& item)) {
        local_map[item.item_id] += item.count;
    }

    // 5. 合并到全局哈希表（加锁，仅合并时加锁，性能极高）
    {
        std::lock_guard<std::mutex> lock(g_map_mutex);
        for (auto& [item_id, cnt] : local_map) {
            g_item_total[item_id] += cnt;
        }
    }

    // 6. 释放资源
    db_free_result(result);
    release_db_connection(conn);

    // 7. 进度打印
    int finished = ++g_finished_shards;
    printf("分表%d 统计完成，总进度：%d/%d\n", shard_id, finished, TOTAL_SHARDS);
}

int main() {
    printf("开始全服物品持有量统计，分表数：%d，最大并发：%d\n", TOTAL_SHARDS, MAX_THREADS);

    // 1. 生成所有分表ID
    std::vector<int> shard_ids;
    for (int i = 0; i < TOTAL_SHARDS; ++i) {
        shard_ids.push_back(i);
    }

    // 2. 线程池调度：分批并行执行，控制并发数
    std::vector<std::thread> threads;
    for (int shard_id : shard_ids) {
        // 等待线程池空位
        while (threads.size() >= MAX_THREADS) {
            for (auto it = threads.begin(); it != threads.end();) {
                if (it->joinable()) {
                    it->join();
                    it = threads.erase(it);
                } else {
                    ++it;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // 启动新线程
        threads.emplace_back(stat_shard, shard_id);
    }

    // 3. 等待所有线程结束
    for (auto& t : threads) {
        t.join();
    }

    // 4. 结果落地：写入运营统计表（不影响在线服务）
    save_stat_result(g_item_total);

    printf("全服统计完成！总物品数：%zu\n", g_item_total.size());
    return 0;
}

// 辅助函数：保存统计结果到结果表
void save_stat_result(const std::unordered_map<uint32_t, uint64_t>& result) {
    DBConn conn = get_admin_db_connection();
    // 清空历史统计数据
    db_execute(conn, "TRUNCATE TABLE item_total_stat");
    
    // 批量插入（高性能）
    for (auto& [item_id, total] : result) {
        char sql[128];
        snprintf(sql, sizeof(sql), 
            "INSERT INTO item_total_stat(item_id, total_count) VALUES(%u, %llu)",
            item_id, (unsigned long long)total);
        db_execute(conn, sql);
    }
    release_db_connection(conn);
}