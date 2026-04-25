#pragma once
#include <cstdint>
#include <memory>
#include "../net/asio_session.h"
#include "../protocol/msg.pb.h"

struct RoleData {
    uint64_t uid;
    std::string name;
    int level;
    int64_t gold;
    int diamond;

    RoleData() : uid(0), level(1), gold(1000), diamond(0) {}
};

class RoleLogic {
public:
    static RoleLogic& Instance();
    void HandleLogin(const msg::LoginReq& req, std::shared_ptr<Session> sess);
    void GetRoleInfo(uint64_t uid, std::shared_ptr<Session> sess);
    bool Load(uint64_t uid, RoleData& out);
    void Save(uint64_t uid, const RoleData& d);
    bool GetPlayerInfo(uint64_t uid, RoleData& out); 
};