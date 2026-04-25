#pragma once
#include <string>
#include <cstdint>

class ShardTool {
public:
    static int RoleTable(uint64_t uid) { return uid % 32; }
    static std::string RoleTableName(uint64_t uid) {
        char buf[32];
        snprintf(buf, 32, "user_role_%02d", RoleTable(uid));
        return buf;
    }
    static std::string ItemTableName(uint64_t uid) {
        char buf[32];
        snprintf(buf, 32, "user_item_%02d", RoleTable(uid));
        return buf;
    }
};