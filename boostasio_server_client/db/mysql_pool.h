#pragma once
#include <mutex>
#include <queue>
#include <mysql/mysql.h>
#include <memory>

class MySQLConn {
public:
    MySQLConn(MYSQL* conn) : conn_(conn) {}
    ~MySQLConn() { mysql_close(conn_); }
    MYSQL* raw() { return conn_; }
private:
    MYSQL* conn_;
};

class MySQLPool {
public:
    static MySQLPool& Instance();
    void Init(const char* host, const char* user, const char* pwd, const char* db, int port);
    std::shared_ptr<MySQLConn> Get();

private:
    MySQLPool() {}
    std::mutex mtx_;
    std::queue<std::shared_ptr<MySQLConn>> pool_;
};