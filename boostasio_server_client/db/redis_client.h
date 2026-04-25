#pragma once
#include <string>
#include <hiredis/hiredis.h>


class RedisClient {
public:
    static RedisClient& Instance();
    bool Connect(const char* host, int port, const std::string& password);
    bool HSet(const std::string& key, const std::string& field, const std::string& val);
    bool HGet(const std::string& key, const std::string& field, std::string& out);
    bool HGetAll(const std::string& key, std::string& out);
    // Redis SET 操作：设置key-value（过期时间可选，单位秒）
    bool Set(const std::string& key, const std::string& value, int expire_seconds = 0);
    // Redis GET 操作：根据key获取value
    std::string Get(const std::string& key);
    bool CheckConnect();
private:
    RedisClient() = default;
    redisContext* ctx_ = nullptr;
private:
    std::string password_;
    std::string ip_;
    std::int32_t port_;
};