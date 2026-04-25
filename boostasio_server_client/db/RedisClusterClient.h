#ifndef REDIS_CLUSTER_CLIENT_H
#define REDIS_CLUSTER_CLIENT_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>

class RedisClusterClient {
public:
    struct NodeConfig {
        std::string host;
        int port;
        std::string password;
        
        NodeConfig(const std::string& h = "127.0.0.1", int p = 7001, const std::string& pass = "")
            : host(h), port(p), password(pass) {}
    };

    static RedisClusterClient& Instance();

    RedisClusterClient();
    ~RedisClusterClient();

    // 初始化集群连接
    bool init(const std::vector<NodeConfig>& nodes, const std::string& password = "", int timeout = 3);
    
    // 关闭所有连接
    void close();
    
    // 基本的键值操作
    bool set(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string& value);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    
    // 带过期时间的set
    bool setex(const std::string& key, const std::string& value, int seconds);
    
    // 批量操作
    bool mget(const std::vector<std::string>& keys, std::map<std::string, std::string>& values);
    bool mset(const std::map<std::string, std::string>& kvPairs);
    
    // 检查连接是否正常
    bool ping();

private:
    struct RedisConnection {
        redisContext* context;
        std::string host;
        int port;
        std::string password;
        std::mutex mutex;
        
        RedisConnection() : context(nullptr), port(0) {}
    };
    
    struct SlotMapping {
        int start;
        int end;
        std::shared_ptr<RedisConnection> master;
    };
    
    // CRC16算法用于计算key的slot
    static unsigned int crc16(const char* buf, int len);
    static unsigned int keyHashSlot(const std::string& key);
    
    // 获取key对应的slot和master连接
    std::shared_ptr<RedisConnection> getConnection(const std::string& key);
    
    // 执行命令的通用方法
    bool executeCommand(const std::string& key, const std::vector<std::string>& args, redisReply*& reply);
    bool executeCommandOnConn(std::shared_ptr<RedisConnection> conn, const std::vector<std::string>& args, redisReply*& reply);
    
    // 刷新集群拓扑
    bool refreshClusterTopology();
    
    // 连接到指定节点
    std::shared_ptr<RedisConnection> connectNode(const NodeConfig& node);
    
    // 解析集群节点信息
    bool parseClusterNodes(const std::string& nodesInfo);
    
    // 处理MOVED重定向
    bool handleMoved(redisReply* reply, const std::vector<std::string>& args);
    
private:
    std::vector<std::shared_ptr<RedisConnection>> m_connections;
    std::map<int, std::shared_ptr<RedisConnection>> m_slotToMaster;  // slot -> master连接
    std::unordered_map<std::string, std::shared_ptr<RedisConnection>> m_addrToConn; // ip:port -> 连接
    std::string m_password;
    std::mutex m_mutex;
    bool m_initialized;
};

#endif // REDIS_CLUSTER_CLIENT_H