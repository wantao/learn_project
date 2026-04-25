#pragma once
#include <mutex>
#include <vector>
#include <string>
#include <cstdint>
#include <thread>
#include <functional>

struct DBTask {
    uint64_t uid;
    std::string sql;
    std::function<void(bool)> cb;
};

class AsyncDB {
public:
    static AsyncDB& Instance();  // 返回引用
    void Init();
    void Push(uint64_t uid, const std::string& sql, std::function<void(bool)> cb = nullptr);

private:
    AsyncDB() : running_(true) {}
    void Loop();
    void Flush();

    std::mutex mtx_;
    std::vector<DBTask> tasks_;
    bool running_;
};