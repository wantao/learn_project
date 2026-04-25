#include "mysql_pool.h"

MySQLPool& MySQLPool::Instance() {
    static MySQLPool ins;
    return ins;
}

void MySQLPool::Init(const char* host, const char* user, const char* pwd, const char* db, int port) {
    for (int i = 0; i < 16; ++i) {
        MYSQL* c = mysql_init(0);
        if (mysql_real_connect(c, host, user, pwd, db, port, 0, 0)) {
            pool_.push(std::shared_ptr<MySQLConn>(new MySQLConn(c)));
        }
    }
}

std::shared_ptr<MySQLConn> MySQLPool::Get() {
    std::lock_guard<std::mutex> lk(mtx_);
    if (pool_.empty()) return std::shared_ptr<MySQLConn>();
    auto conn = pool_.front();
    pool_.pop();
    return conn;
}