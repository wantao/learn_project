#include "role_logic.h"
#include "../shard/shard_tool.h"
#include "../db/RedisClusterClient.h"
#include "../async/async_db.h"
#include "../async/SyncDB.h"

RoleLogic& RoleLogic::Instance() {
    static RoleLogic ins;
    return ins;
}

bool RoleLogic::Load(uint64_t uid, RoleData& out) {
    GetPlayerInfo(uid, out);

    return true;
}

void RoleLogic::Save(uint64_t uid, const RoleData& d) {
    std::string table = ShardTool::RoleTableName(uid);
    char sql[512];
    snprintf(sql, 512, "REPLACE INTO %s (uid,name,level,gold,diamond) VALUES (%lu,'%s',%d,%ld,%d)",
        table.c_str(), uid, d.name.c_str(), d.level, d.gold, d.diamond);
    AsyncDB::Instance().Push(uid, sql);
}


void RoleLogic::HandleLogin(const msg::LoginReq& req, std::shared_ptr<Session> sess) {   
    uint64_t uid = 10001 + rand() % 1000;
    GetRoleInfo(uid, sess);
    sess->set_uid(uid);
    msg::LoginAck ack;
    ack.set_code(0);
    ack.set_uid(uid);
    sess->Send(1002, ack);
}

void RoleLogic::GetRoleInfo(uint64_t uid, std::shared_ptr<Session> sess) {
    RoleData d;
    Load(uid, d);
    msg::RoleInfoAck ack;
    ack.set_uid(d.uid);
    ack.set_name(d.name);
    ack.set_level(d.level);
    ack.set_gold(d.gold);
    ack.set_diamond(d.diamond);
    sess->Send(1004, ack);
}

bool RoleLogic::GetPlayerInfo(uint64_t uid, RoleData& out) {
    out.uid = uid;
    
    // First try to load from Redis
    std::string key = "role:" + std::to_string(uid);
    std::string value;
    RedisClusterClient::Instance().get(key, value);
    if (!value.empty()) {
        // Parse from Redis value (assume format: name,level,gold,diamond)
        std::stringstream ss(value);
        std::string token;
        std::getline(ss, token, ',');
        out.name = token;
        std::getline(ss, token, ',');
        out.level = std::stoi(token);
        std::getline(ss, token, ',');
        out.gold = std::stol(token);
        std::getline(ss, token, ',');
        out.diamond = std::stoi(token);
        std::cout << "1111Loaded player info from Redis for uid: " << uid << std::endl<<" key:"<<key<<std::endl; 
        return true;
    }
    
    // If not in Redis, load from DB
    std::string table = ShardTool::RoleTableName(uid);
    char sql[512];
    snprintf(sql, 512, "SELECT name,level,gold,diamond FROM %s WHERE uid = %lu", table.c_str(), uid);
    std::string result = SyncDB::Instance().QuerySql(sql);
    if (!result.empty()) {
        std::stringstream ss(result);
        std::string token;
        std::getline(ss, token, ',');
        out.name = token;
        std::getline(ss, token, ',');
        out.level = std::stoi(token);
        std::getline(ss, token, ',');
        out.gold = std::stol(token);
        std::getline(ss, token, ',');
        out.diamond = std::stoi(token);
        
        // Write back to Redis
        std::string redis_value = out.name + "," + std::to_string(out.level) + "," + std::to_string(out.gold) + "," + std::to_string(out.diamond);
        RedisClusterClient::Instance().set(key, redis_value);
        std::cout << "222Loaded player info from Redis for redis_value: " << redis_value << std::endl<<" key:"<<key<<std::endl; 
        return true;
    }
    
    // If not in DB, create player account and role info in DB
    out.name = "test_user";
    out.level = 1;
    out.gold = 0;
    out.diamond = 0;
    
    // Insert into DB
    char insert_sql[512];
    snprintf(insert_sql, 512, "INSERT INTO %s (uid,name,level,gold,diamond) VALUES (%lu,'%s',%d,%ld,%d)",
        table.c_str(), uid, out.name.c_str(), out.level, out.gold, out.diamond);
    AsyncDB::Instance().Push(uid, insert_sql);
    
    // Write back to Redis
    std::string redis_value = out.name + "," + std::to_string(out.level) + "," + std::to_string(out.gold) + "," + std::to_string(out.diamond);
    RedisClusterClient::Instance().set(key, redis_value);
    std::cout << "3333Loaded player info from Redis for redis_value: " << redis_value << std::endl<<" insert_sql:"<<insert_sql<<std::endl; 
    return true;
}