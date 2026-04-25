#ifndef SYNC_DB_H
#define SYNC_DB_H

#include <string>
#include <vector>

// 数据库节点类型
enum DBNodeType
{
    MASTER = 0,   // 主库（写）
    SLAVE1 = 1,   // 从库1（读）
    SLAVE2 = 2    // 从库2（读）
};

// 一主两从 读写分离数据库类
class SyncDB
{
public:
    static SyncDB& Instance();

    SyncDB(const SyncDB&) = delete;
    SyncDB& operator=(const SyncDB&) = delete;

    // 你原有接口：完全兼容
    static std::string Query(const std::string& sql);

    // 执行SQL（写操作：INSERT/UPDATE/DELETE）
    bool Execute(const std::string& sql);

    // 查询SQL（读操作：自动走从库）
    std::string QuerySql(const std::string& sql);

private:
    SyncDB();
    ~SyncDB();

    bool Connect(DBNodeType type);
    void Disconnect(DBNodeType type);
    bool CheckConnect(DBNodeType type);

    // 自动判断SQL类型：读/写
    bool IsSelectSql(const std::string& sql);

    // 读负载均衡：轮询从库
    DBNodeType GetNextSlave();

private:
    // 三个数据库连接：主 + 从1 + 从2
    void* conn_[3];
    int slave_index_; // 轮询索引
};

#endif