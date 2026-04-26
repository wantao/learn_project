#include <iostream>
#include <string>
#include <memcached.h>
#include <string.h>

// ===================== 集群配置 =====================
#define MEMCACHED_CLUSTER_NODES "192.168.72.131:11211,192.168.72.131:11212,192.168.72.131:11213,192.168.72.131:11214"
#define CACHE_DEFAULT_EXPIRE 3600  // 默认过期时间1小时（游戏会话）
#define CACHE_TIMEOUT 500          // 超时500ms（游戏低延迟要求）

// Memcached集群句柄
static memcached_st* g_memc = nullptr;

// ===================== 集群初始化 =====================
bool memcached_cluster_init() {
    // 创建句柄
    g_memc = memcached_create(nullptr);
    if (!g_memc) {
        std::cerr << "memcached create failed" << std::endl;
        return false;
    }

    // 配置：一致性哈希（Ketama）= 集群核心
    memcached_behavior_set(g_memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA);
    memcached_behavior_set(g_memc, MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS, 1);  // 自动剔除故障节点
    memcached_behavior_set(g_memc, MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT, CACHE_TIMEOUT);
    memcached_behavior_set(g_memc, MEMCACHED_BEHAVIOR_RCV_TIMEOUT, CACHE_TIMEOUT);
    memcached_behavior_set(g_memc, MEMCACHED_BEHAVIOR_SND_TIMEOUT, CACHE_TIMEOUT);
    memcached_behavior_set(g_memc, MEMCACHED_BEHAVIOR_TCP_NODELAY, 1);  // 禁用Nagle，低延迟

    // 解析集群节点
    memcached_server_st* servers = memcached_servers_parse(MEMCACHED_CLUSTER_NODES);
    if (!servers) {
        std::cerr << "parse servers failed" << std::endl;
        memcached_free(g_memc);
        g_memc = nullptr;
        return false;
    }

    // 挂载节点
    memcached_server_push(g_memc, servers);
    memcached_server_list_free(servers);

    std::cout << "memcached cluster init success" << std::endl;
    return true;
}

// ===================== 缓存写入 =====================
bool memcached_set(const char* key, const char* value, time_t expire = CACHE_DEFAULT_EXPIRE) {
    if (!g_memc || !key || !value) return false;

    memcached_return_t rc = memcached_set(
        g_memc,
        key, strlen(key),
        value, strlen(value),
        expire,
        0  // flags
    );

    return rc == MEMCACHED_SUCCESS;
}

// ===================== 缓存读取 =====================
std::string memcached_get(const char* key) {
    if (!g_memc || !key) return "";

    size_t value_len = 0;
    uint32_t flags = 0;
    memcached_return_t rc;

    char* value = memcached_get(
        g_memc,
        key, strlen(key),
        &value_len, &flags, &rc
    );

    if (rc == MEMCACHED_SUCCESS && value) {
        std::string res(value, value_len);
        free(value);
        return res;
    }
    return "";
}

// ===================== 缓存删除 =====================
bool memcached_del(const char* key) {
    if (!g_memc || !key) return false;
    memcached_return_t rc = memcached_delete(g_memc, key, strlen(key), 0);
    return rc == MEMCACHED_SUCCESS;
}

// ===================== 集群关闭 =====================
void memcached_cluster_close() {
    if (g_memc) {
        memcached_free(g_memc);
        g_memc = nullptr;
    }
}

// ===================== 测试主函数 =====================
int main() {
    // 初始化集群
    if (!memcached_cluster_init()) {
        return -1;
    }

    // 游戏业务：存储玩家数据
    const char* key = "game:player:10001";
    const char* value = R"({"uid":10001,"name":"test","level":50,"gold":9999})";

    // 写入缓存
    if (memcached_set(key, value)) {
        std::cout << "set success" << std::endl;
    }

    // 读取缓存
    std::string res = memcached_get(key);
    std::cout << "get value: " << res << std::endl;

    // 删除缓存
    // memcached_del(key);

    memcached_cluster_close();
    return 0;
}