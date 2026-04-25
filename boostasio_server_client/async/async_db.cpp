#include "async_db.h"
#include "../db/mysql_pool.h"

AsyncDB& AsyncDB::Instance() {
    static AsyncDB ins;
    return ins;
}

void AsyncDB::Init() {
    std::thread(&AsyncDB::Loop, this).detach();
}

void AsyncDB::Push(uint64_t uid, const std::string& sql, std::function<void(bool)> cb) {
    std::lock_guard<std::mutex> lk(mtx_);
    tasks_.push_back({uid, sql, cb});
}

void AsyncDB::Loop() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Flush();
    }
}

void AsyncDB::Flush() {
    std::lock_guard<std::mutex> lk(mtx_);
    if (tasks_.empty()) return;

    auto conn = MySQLPool::Instance().Get();
    if (!conn) return;

    mysql_query(conn->raw(), "START TRANSACTION");
    bool ok = true;

    for (auto& t : tasks_) {
        if (mysql_query(conn->raw(), t.sql.c_str())) { ok = false; break; }
    }

    if (ok) mysql_query(conn->raw(), "COMMIT");
    else mysql_query(conn->raw(), "ROLLBACK");

    for (auto& t : tasks_) if (t.cb) t.cb(ok);
    tasks_.clear();
}