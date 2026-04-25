#pragma once
#include <boost/asio.hpp>
#include "protocol/msg.pb.h"

using namespace boost::asio;

class ProtobufCodec
{
public:
    // 打包：Protobuf → 二进制流
    static std::string Encode(uint32_t cmd, uint64_t uid, const google::protobuf::Message& msg)
    {
        msg::Packet pack;
        pack.set_cmd(cmd);
        pack.set_uid(uid);
        pack.set_body(msg.SerializeAsString());

        std::string data = pack.SerializeAsString();
        uint32_t len = data.size();
        uint32_t network_len = htonl(len);

        std::string packet;
        packet.append((const char*)&network_len, 4);  // 4字节长度头
        packet.append(data);
        return packet;
    }

    // 解包：二进制流 → Protobuf
    static bool Decode(const char* data, uint32_t len, msg::Packet& pack)
    {
        return pack.ParseFromArray(data, len);
    }
};