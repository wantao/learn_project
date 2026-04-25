#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <memory>
#include "protobuf_codec.h"
#include "protocol/msg.pb.h"

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket);
    void Start();

    template <typename T>
    void Send(uint32_t cmd, const T& msg) {
        std::string packet = ProtobufCodec::Encode(cmd, uid_, msg);
        boost::asio::async_write(socket_,
            boost::asio::buffer(packet),
            boost::bind(&Session::HandleSend,
                shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    void HandleSend(const boost::system::error_code&, size_t);

    uint64_t uid() const { return uid_; }
    void set_uid(uint64_t uid) { uid_ = uid; }

private:
    void ReadLength();
    void ReadBody(uint32_t len);
    void OnPacket(const char* data, uint32_t len);
    void Dispatch(uint32_t cmd, const msg::Packet& p);

    tcp::socket socket_;
    char read_buf_[4096];
    uint64_t uid_;
};