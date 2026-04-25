#include "redis_client.h"
#include <iostream>
#include <cstring>

RedisClient& RedisClient::Instance() {
    static RedisClient ins;
    return ins;
}

bool RedisClient::Connect(const char* host, int port, const std::string& password) {
    ctx_ = redisConnect(host, port);
    password_ = password;
    if (ctx_ && !ctx_->err) {
        if (!password_.empty()) {
            redisReply* reply = (redisReply*)redisCommand(ctx_, "AUTH %s", password_.c_str());
            if (!reply || reply->type == REDIS_REPLY_ERROR) {
                std::cerr << "Redis auth failed: " << reply->str << std::endl;
                freeReplyObject(reply);
                redisFree(ctx_);
                ctx_ = nullptr;
                return false;
            }
            freeReplyObject(reply);    
        }
        return true;
    }
    return false;
}

bool RedisClient::HSet(const std::string& key, const std::string& field, const std::string& val) {
    auto r = (redisReply*)redisCommand(ctx_, "HSET %s %s %s", key.c_str(), field.c_str(), val.c_str());
    bool ok = r && r->type == REDIS_REPLY_INTEGER;
    freeReplyObject(r);
    return ok;
}

bool RedisClient::HGet(const std::string& key, const std::string& field, std::string& out) {
    auto r = (redisReply*)redisCommand(ctx_, "HGET %s %s", key.c_str(), field.c_str());
    if (r && r->type == REDIS_REPLY_STRING) { out = r->str; freeReplyObject(r); return true; }
    freeReplyObject(r);
    return false;
}

bool RedisClient::HGetAll(const std::string& key, std::string& out) {
    auto r = (redisReply*)redisCommand(ctx_, "HGETALL %s", key.c_str());
    if (r && r->type == REDIS_REPLY_ARRAY) { out = "{}"; freeReplyObject(r); return true; }
    freeReplyObject(r);
    return false;
}

// 检查/重连Redis
bool RedisClient::CheckConnect() {
    // 如果连接失效，重新建立连接
    if (!ctx_ || ctx_->err) {
        if (ctx_) {
            redisFree(ctx_);
            ctx_ = nullptr;
        }
        // 连接Redis服务器
        ctx_ = redisConnect(ip_.c_str(), port_);
        if (!ctx_ || ctx_->err) {
            std::cerr << "Redis connect failed: " << (ctx_ ? ctx_->errstr : "nullptr") << std::endl;
            return false;
        }

        // 如果Redis有密码，执行AUTH认证
        if (!password_.empty()) {
            redisReply* reply = (redisReply*)redisCommand(ctx_, "AUTH %s", password_.c_str());
            if (!reply || reply->type == REDIS_REPLY_ERROR) {
                std::cerr << "Redis auth failed: " << reply->str << std::endl;
                freeReplyObject(reply);
                redisFree(ctx_);
                ctx_ = nullptr;
                return false;
            }
            freeReplyObject(reply);
        }
    }
    return true;
}

// Redis SET 实现
bool RedisClient::Set(const std::string& key, const std::string& value, int expire_seconds) {
    if (!CheckConnect()) {
        return false;
    }

    redisReply* reply = nullptr;
    if (expire_seconds > 0) {
        // SET key value EX 过期时间
        reply = (redisReply*)redisCommand(ctx_, "SET %s %s EX %d", 
                                          key.c_str(), value.c_str(), expire_seconds);
    } else {
        // 普通SET
        reply = (redisReply*)redisCommand(ctx_, "SET %s %s", 
                                          key.c_str(), value.c_str());
    }

    // 检查执行结果
    bool ret = false;
    if (reply && reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0) {
        ret = true;
    } else {
        std::cerr << "Redis SET failed: " << (reply ? reply->str : "nullptr") << std::endl;
    }

    // 释放回复内存（必须！否则内存泄漏）
    if (reply) {
        freeReplyObject(reply);
    }
    return ret;
}

// Redis GET 实现
std::string RedisClient::Get(const std::string& key) {
    if (!CheckConnect()) {
        return "";
    }

    // 执行GET命令
    redisReply* reply = (redisReply*)redisCommand(ctx_, "GET %s", key.c_str());
    std::string result;

    // 处理返回结果
    if (reply) {
        if (reply->type == REDIS_REPLY_STRING) {
            result = reply->str; // 有值则返回字符串
        } else if (reply->type == REDIS_REPLY_NIL) {
            result = ""; // key不存在返回空
        } else {
            std::cerr << "Redis GET failed: " << reply->str << std::endl;
        }
        freeReplyObject(reply); // 释放回复内存
    } else {
        std::cerr << "Redis GET reply is nullptr" << std::endl;
    }

    return result;
}