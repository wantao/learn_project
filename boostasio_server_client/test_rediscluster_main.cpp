#include "RedisClusterClient.h"
#include <iostream>

int main() {
    RedisClusterClient client;
    
    // 配置集群节点
    std::vector<RedisClusterClient::NodeConfig> nodes = {
        {"127.0.0.1", 7001, "Redis@123"},
        {"127.0.0.1", 7002, "Redis@123"},
        {"127.0.0.1", 7003, "Redis@123"}
    };
    
    // 初始化集群
    if (!client.init(nodes, "Redis@123")) {
        std::cerr << "Failed to initialize Redis cluster" << std::endl;
        return -1;
    }
    
    // 测试SET
    if (client.set("test_key", "test_value")) {
        std::cout << "SET success" << std::endl;
    } else {
        std::cerr << "SET failed" << std::endl;
    }
    
    // 测试GET
    std::string value;
    if (client.get("test_key", value)) {
        std::cout << "GET success: " << value << std::endl;
    } else {
        std::cerr << "GET failed" << std::endl;
    }
    
    // 测试带过期时间的SET
    if (client.setex("expire_key", "expire_value", 60)) {
        std::cout << "SETEX success" << std::endl;
    }
    
    // 测试EXISTS
    if (client.exists("test_key")) {
        std::cout << "Key exists" << std::endl;
    }
    
    // 测试批量操作
    std::map<std::string, std::string> kvPairs = {
        {"key1", "value1"},
        {"key2", "value2"},
        {"key3", "value3"}
    };
    
    if (client.mset(kvPairs)) {
        std::cout << "MSET success" << std::endl;
    }
    
    std::vector<std::string> keys = {"key1", "key2", "key3"};
    std::map<std::string, std::string> values;
    if (client.mget(keys, values)) {
        std::cout << "MGET success:" << std::endl;
        for (const auto& pair : values) {
            std::cout << "  " << pair.first << " -> " << pair.second << std::endl;
        }
    }
    
    // 测试DELETE
    if (client.del("test_key")) {
        std::cout << "DEL success" << std::endl;
    }
    
    // 关闭连接
    client.close();
    
    return 0;
}