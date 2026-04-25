#include "RedisClusterClient.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>


// CRC16表
static const unsigned short crc16tab[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

RedisClusterClient& RedisClusterClient::Instance() {
    static RedisClusterClient ins;
    return ins;
}

unsigned int RedisClusterClient::crc16(const char* buf, int len) {
    unsigned int crc = 0;
    for (int i = 0; i < len; i++) {
        crc = (crc << 8) ^ crc16tab[((crc >> 8) ^ *buf++) & 0x00FF];
    }
    return crc & 0xFFFF;
}

unsigned int RedisClusterClient::keyHashSlot(const std::string& key) {
    // 实现 Redis 集群的 key hash slot 算法
    size_t keylen = key.length();
    const char* keydata = key.c_str();
    
    size_t s = key.find('{');
    if (s != std::string::npos) {
        size_t e = key.find('}', s + 1);
        if (e != std::string::npos && e != s + 1) {
            keydata = keydata + s + 1;
            keylen = e - s - 1;
        }
    }
    
    return crc16(keydata, keylen) & 0x3FFF; // 16384 slots
}

RedisClusterClient::RedisClusterClient() : m_initialized(false) {
}

RedisClusterClient::~RedisClusterClient() {
    close();
}

bool RedisClusterClient::init(const std::vector<NodeConfig>& nodes, const std::string& password, int timeout) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_password = password;
    
    // 连接到所有节点
    for (const auto& node : nodes) {
        auto conn = connectNode(node);
        if (conn && conn->context) {
            m_connections.push_back(conn);
            std::string addr = node.host + ":" + std::to_string(node.port);
            m_addrToConn[addr] = conn;
        }
    }
    
    if (m_connections.empty()) {
        std::cerr << "Failed to connect to any Redis node" << std::endl;
        return false;
    }
    
    // 刷新集群拓扑
    if (!refreshClusterTopology()) {
        std::cerr << "Failed to refresh cluster topology" << std::endl;
        return false;
    }
    
    m_initialized = true;
    return true;
}

std::shared_ptr<RedisClusterClient::RedisConnection> RedisClusterClient::connectNode(const NodeConfig& node) {
    auto conn = std::make_shared<RedisConnection>();
    conn->host = node.host;
    conn->port = node.port;
    conn->password = node.password.empty() ? m_password : node.password;
    
    struct timeval timeout = {3, 0};
    conn->context = redisConnectWithTimeout(node.host.c_str(), node.port, timeout);
    
    if (!conn->context || conn->context->err) {
        if (conn->context) {
            std::cerr << "Connection error: " << conn->context->errstr << std::endl;
            redisFree(conn->context);
            conn->context = nullptr;
        } else {
            std::cerr << "Connection error: can't allocate redis context" << std::endl;
        }
        return nullptr;
    }
    
    // 认证
    if (!conn->password.empty()) {
        redisReply* reply = (redisReply*)redisCommand(conn->context, "AUTH %s", conn->password.c_str());
        if (!reply || reply->type == REDIS_REPLY_ERROR) {
            std::cerr << "Auth failed: " << (reply ? reply->str : "unknown error") << std::endl;
            if (reply) freeReplyObject(reply);
            redisFree(conn->context);
            conn->context = nullptr;
            return nullptr;
        }
        freeReplyObject(reply);
    }
    
    return conn;
}

bool RedisClusterClient::refreshClusterTopology() {
    for (auto& conn : m_connections) {
        redisReply* reply = (redisReply*)redisCommand(conn->context, "CLUSTER NODES");
        if (reply && reply->type == REDIS_REPLY_STRING) {
            bool result = parseClusterNodes(reply->str);
            freeReplyObject(reply);
            if (result) {
                return true;
            }
        }
        if (reply) freeReplyObject(reply);
    }
    return false;
}

bool RedisClusterClient::parseClusterNodes(const std::string& nodesInfo) {
    std::map<int, std::shared_ptr<RedisConnection>> newSlotMapping;
    std::istringstream iss(nodesInfo);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (line.empty()) continue;
        
        std::vector<std::string> parts;
        size_t pos = 0;
        std::string token;
        while ((pos = line.find(' ')) != std::string::npos) {
            token = line.substr(0, pos);
            parts.push_back(token);
            line.erase(0, pos + 1);
        }
        parts.push_back(line);
        
        if (parts.size() < 8) continue;
        
        // parts[0]: nodeid, parts[1]: ip:port, parts[2]: flags
        std::string flags = parts[2];
        bool isMaster = flags.find("master") != std::string::npos;
        
        if (!isMaster) continue;
        
        // 解析 ip:port
        std::string addr = parts[1];
        size_t colonPos = addr.find(':');
        if (colonPos == std::string::npos) continue;
        
        std::string host = addr.substr(0, colonPos);
        int port = std::stoi(addr.substr(colonPos + 1));
        
        // 查找或创建连接
        std::string fullAddr = host + ":" + std::to_string(port);
        auto it = m_addrToConn.find(fullAddr);
        std::shared_ptr<RedisConnection> masterConn;
        
        if (it != m_addrToConn.end()) {
            masterConn = it->second;
        } else {
            // 创建新连接
            NodeConfig node(host, port, m_password);
            masterConn = connectNode(node);
            if (masterConn && masterConn->context) {
                m_addrToConn[fullAddr] = masterConn;
                m_connections.push_back(masterConn);
            } else {
                continue;
            }
        }
        
        // 解析 slots
        for (size_t i = 8; i < parts.size(); i++) {
            const std::string& slotInfo = parts[i];
            if (slotInfo.empty() || slotInfo[0] == '[') continue;
            
            size_t dashPos = slotInfo.find('-');
            if (dashPos != std::string::npos) {
                int start = std::stoi(slotInfo.substr(0, dashPos));
                int end = std::stoi(slotInfo.substr(dashPos + 1));
                for (int slot = start; slot <= end; slot++) {
                    newSlotMapping[slot] = masterConn;
                }
            } else {
                int slot = std::stoi(slotInfo);
                newSlotMapping[slot] = masterConn;
            }
        }
    }
    
    if (!newSlotMapping.empty()) {
        m_slotToMaster = newSlotMapping;
        return true;
    }
    
    return false;
}

std::shared_ptr<RedisClusterClient::RedisConnection> RedisClusterClient::getConnection(const std::string& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        return nullptr;
    }
    
    unsigned int slot = keyHashSlot(key);
    auto it = m_slotToMaster.find(slot);
    
    if (it != m_slotToMaster.end() && it->second && it->second->context) {
        return it->second;
    }
    
    // 如果slot映射不存在，刷新拓扑
    refreshClusterTopology();
    it = m_slotToMaster.find(slot);
    if (it != m_slotToMaster.end() && it->second && it->second->context) {
        return it->second;
    }
    
    // 如果还是找不到，返回第一个可用连接
    if (!m_connections.empty() && m_connections[0]->context) {
        return m_connections[0];
    }
    
    return nullptr;
}

bool RedisClusterClient::executeCommandOnConn(std::shared_ptr<RedisConnection> conn, 
                                              const std::vector<std::string>& args, 
                                              redisReply*& reply) {
    if (!conn || !conn->context) return false;
    
    std::lock_guard<std::mutex> lock(conn->mutex);
    
    const char** argv = new const char*[args.size()];
    size_t* argvlen = new size_t[args.size()];
    
    for (size_t i = 0; i < args.size(); i++) {
        argv[i] = args[i].c_str();
        argvlen[i] = args[i].length();
    }
    
    reply = (redisReply*)redisCommandArgv(conn->context, args.size(), argv, argvlen);
    
    delete[] argv;
    delete[] argvlen;
    
    return reply != nullptr;
}

bool RedisClusterClient::executeCommand(const std::string& key, 
                                       const std::vector<std::string>& args, 
                                       redisReply*& reply) {
    auto conn = getConnection(key);
    if (!conn) {
        return false;
    }
    
    if (!executeCommandOnConn(conn, args, reply)) {
        return false;
    }
    
    // 处理 MOVED 重定向
    if (reply && reply->type == REDIS_REPLY_ERROR) {
        std::string errMsg(reply->str);
        if (errMsg.find("MOVED") == 0) {
            freeReplyObject(reply);
            return handleMoved(reply, args);
        }
    }
    
    return true;
}

bool RedisClusterClient::handleMoved(redisReply* reply, const std::vector<std::string>& args) {
    // 解析 MOVED 响应，格式: "MOVED slot ip:port"
    std::string errMsg(reply->str);
    size_t space1 = errMsg.find(' ');
    size_t space2 = errMsg.find(' ', space1 + 1);
    
    if (space1 != std::string::npos && space2 != std::string::npos) {
        std::string addr = errMsg.substr(space2 + 1);
        size_t colonPos = addr.find(':');
        if (colonPos != std::string::npos) {
            std::string host = addr.substr(0, colonPos);
            int port = std::stoi(addr.substr(colonPos + 1));
            
            // 连接到新节点
            NodeConfig node(host, port, m_password);
            auto newConn = connectNode(node);
            if (newConn && newConn->context) {
                std::string fullAddr = host + ":" + std::to_string(port);
                m_addrToConn[fullAddr] = newConn;
                m_connections.push_back(newConn);
                
                // 重试命令
                redisReply* newReply = nullptr;
                bool result = executeCommandOnConn(newConn, args, newReply);
                if (result && newReply) {
                    reply = newReply;
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool RedisClusterClient::set(const std::string& key, const std::string& value) {
    redisReply* reply = nullptr;
    std::vector<std::string> args = {"SET", key, value};
    
    if (!executeCommand(key, args, reply)) {
        return false;
    }
    
    bool success = (reply && reply->type == REDIS_REPLY_STATUS && 
                    std::string(reply->str) == "OK");
    
    if (reply) freeReplyObject(reply);
    return success;
}

bool RedisClusterClient::get(const std::string& key, std::string& value) {
    redisReply* reply = nullptr;
    std::vector<std::string> args = {"GET", key};
    
    if (!executeCommand(key, args, reply)) {
        return false;
    }
    
    bool success = false;
    if (reply && reply->type == REDIS_REPLY_STRING) {
        value = std::string(reply->str, reply->len);
        success = true;
    } else if (reply && reply->type == REDIS_REPLY_NIL) {
        value.clear();
        success = true;
    }
    
    if (reply) freeReplyObject(reply);
    return success;
}

bool RedisClusterClient::del(const std::string& key) {
    redisReply* reply = nullptr;
    std::vector<std::string> args = {"DEL", key};
    
    if (!executeCommand(key, args, reply)) {
        return false;
    }
    
    bool success = (reply && reply->type == REDIS_REPLY_INTEGER);
    
    if (reply) freeReplyObject(reply);
    return success;
}

bool RedisClusterClient::exists(const std::string& key) {
    redisReply* reply = nullptr;
    std::vector<std::string> args = {"EXISTS", key};
    
    if (!executeCommand(key, args, reply)) {
        return false;
    }
    
    bool exists = (reply && reply->type == REDIS_REPLY_INTEGER && reply->integer == 1);
    
    if (reply) freeReplyObject(reply);
    return exists;
}

bool RedisClusterClient::setex(const std::string& key, const std::string& value, int seconds) {
    redisReply* reply = nullptr;
    std::vector<std::string> args = {"SETEX", key, std::to_string(seconds), value};
    
    if (!executeCommand(key, args, reply)) {
        return false;
    }
    
    bool success = (reply && reply->type == REDIS_REPLY_STATUS && 
                    std::string(reply->str) == "OK");
    
    if (reply) freeReplyObject(reply);
    return success;
}

bool RedisClusterClient::mget(const std::vector<std::string>& keys, 
                              std::map<std::string, std::string>& values) {
    // 简化实现：按slot分组，分别请求
    std::map<std::shared_ptr<RedisConnection>, std::vector<std::string>> groupKeys;
    
    for (const auto& key : keys) {
        auto conn = getConnection(key);
        if (conn) {
            groupKeys[conn].push_back(key);
        }
    }
    
    values.clear();
    bool allSuccess = true;
    
    for (const auto& group : groupKeys) {
        std::vector<std::string> args = {"MGET"};
        args.insert(args.end(), group.second.begin(), group.second.end());
        
        redisReply* reply = nullptr;
        if (executeCommandOnConn(group.first, args, reply) && reply && reply->type == REDIS_REPLY_ARRAY) {
            for (size_t i = 0; i < reply->elements && i < group.second.size(); i++) {
                if (reply->element[i] && reply->element[i]->type == REDIS_REPLY_STRING) {
                    values[group.second[i]] = std::string(reply->element[i]->str, reply->element[i]->len);
                }
            }
        } else {
            allSuccess = false;
        }
        
        if (reply) freeReplyObject(reply);
    }
    
    return allSuccess;
}

bool RedisClusterClient::mset(const std::map<std::string, std::string>& kvPairs) {
    // 简化实现：按slot分组，分别执行MSET
    std::map<std::shared_ptr<RedisConnection>, std::vector<std::string>> groupArgs;
    
    for (const auto& pair : kvPairs) {
        auto conn = getConnection(pair.first);
        if (conn) {
            groupArgs[conn].push_back(pair.first);
            groupArgs[conn].push_back(pair.second);
        }
    }
    
    bool allSuccess = true;
    
    for (const auto& group : groupArgs) {
        std::vector<std::string> args = {"MSET"};
        args.insert(args.end(), group.second.begin(), group.second.end());
        
        redisReply* reply = nullptr;
        if (executeCommandOnConn(group.first, args, reply) && reply && 
            reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK") {
            // success
        } else {
            allSuccess = false;
        }
        
        if (reply) freeReplyObject(reply);
    }
    
    return allSuccess;
}

bool RedisClusterClient::ping() {
    for (auto& conn : m_connections) {
        redisReply* reply = (redisReply*)redisCommand(conn->context, "PING");
        if (reply) {
            bool ok = (reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "PONG");
            freeReplyObject(reply);
            if (ok) return true;
        }
    }
    return false;
}

void RedisClusterClient::close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto& conn : m_connections) {
        if (conn && conn->context) {
            redisFree(conn->context);
            conn->context = nullptr;
        }
    }
    m_connections.clear();
    m_slotToMaster.clear();
    m_addrToConn.clear();
    m_initialized = false;
}