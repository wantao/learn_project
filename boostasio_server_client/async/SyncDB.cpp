#include "SyncDB.h"
#include <mysql/mysql.h>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <sstream>

// ====================== 一主两从配置 ======================
#define MASTER_HOST     "127.0.0.1"
#define MASTER_USER     "root"
#define MASTER_PWD      "Wt315771557#"
#define MASTER_DB       "game_user"
#define MASTER_PORT     3306

#define SLAVE1_HOST     "127.0.0.1"
#define SLAVE1_USER     "root"
#define SLAVE1_PWD      "Wt315771557#"
#define SLAVE1_DB       "game_user"
#define SLAVE1_PORT     3307

#define SLAVE2_HOST     "127.0.0.1"
#define SLAVE2_USER     "root"
#define SLAVE2_PWD      "Wt315771557#"
#define SLAVE2_DB       "game_user"
#define SLAVE2_PORT     3308
// ==========================================================

SyncDB::SyncDB() : slave_index_(0)
{
    for (int i = 0; i < 3; ++i)
        conn_[i] = nullptr;

    Connect(MASTER);
    Connect(SLAVE1);
    Connect(SLAVE2);
}

SyncDB::~SyncDB()
{
    Disconnect(MASTER);
    Disconnect(SLAVE1);
    Disconnect(SLAVE2);
}

SyncDB& SyncDB::Instance()
{
    static SyncDB instance;
    return instance;
}

bool SyncDB::Connect(DBNodeType type)
{
    const char* host;
    const char* user;
    const char* pwd;
    const char* db;
    int port;

    if (type == MASTER)
    {
        host = MASTER_HOST;
        user = MASTER_USER;
        pwd = MASTER_PWD;
        db = MASTER_DB;
        port = MASTER_PORT;
    }
    else if (type == SLAVE1)
    {
        host = SLAVE1_HOST;
        user = SLAVE1_USER;
        pwd = SLAVE1_PWD;
        db = SLAVE1_DB;
        port = SLAVE1_PORT;
    }
    else
    {
        host = SLAVE2_HOST;
        user = SLAVE2_USER;
        pwd = SLAVE2_PWD;
        db = SLAVE2_DB;
        port = SLAVE2_PORT;
    }

    if (conn_[type])
    {
        mysql_close((MYSQL*)conn_[type]);
        conn_[type] = nullptr;
    }

    MYSQL* c = mysql_init(nullptr);
    if (!c) return false;

    unsigned int timeout = 3;
    mysql_options(c, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

    if (!mysql_real_connect(c, host, user, pwd, db, port, nullptr, 0))
    {
        std::cerr << "DB connect failed: " << mysql_error(c) << std::endl;
        mysql_close(c);
        return false;
    }

    mysql_set_character_set(c, "utf8mb4");
    conn_[type] = c;
    return true;
}

void SyncDB::Disconnect(DBNodeType type)
{
    if (conn_[type])
    {
        mysql_close((MYSQL*)conn_[type]);
        conn_[type] = nullptr;
    }
}

bool SyncDB::CheckConnect(DBNodeType type)
{
    if (!conn_[type])
        return Connect(type);

    if (mysql_ping((MYSQL*)conn_[type]) != 0)
        return Connect(type);

    return true;
}

// 自动判断是否为查询语句
bool SyncDB::IsSelectSql(const std::string& sql)
{
    std::string s = sql;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s.find("select") != std::string::npos;
}

// 轮询从库
DBNodeType SyncDB::GetNextSlave()
{
    slave_index_ = (slave_index_ + 1) % 2;
    return slave_index_ == 0 ? SLAVE1 : SLAVE2;
}

// 执行写操作
bool SyncDB::Execute(const std::string& sql)
{
    if (!CheckConnect(MASTER)) return false;
    MYSQL* c = (MYSQL*)conn_[MASTER];

    if (mysql_query(c, sql.c_str()) != 0)
    {
        std::cerr << "Execute error: " << mysql_error(c) << std::endl;
        return false;
    }
    return true;
}

// 查询操作：自动走从库
std::string SyncDB::QuerySql(const std::string& sql)
{
    DBNodeType type = GetNextSlave();
    if (!CheckConnect(type)) return "";

    MYSQL* c = (MYSQL*)conn_[type];
    if (mysql_query(c, sql.c_str()) != 0)
    {
        std::cerr << "Query error: " << mysql_error(c) << std::endl;
        return "";
    }

    MYSQL_RES* res = mysql_store_result(c);
    if (!res) return "";

    std::string result;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row && row[0])
    {
        std::stringstream ss;
        ss << (row[0] ? row[0] : "") << ","
           << (row[1] ? row[1] : "0") << ","
           << (row[2] ? row[2] : "0") << ","
           << (row[3] ? row[3] : "0");
        result = ss.str();
    }
    mysql_free_result(res);
    return result;
}

// 静态接口：你原有代码完全兼容
std::string SyncDB::Query(const std::string& sql)
{
    return Instance().QuerySql(sql);
}