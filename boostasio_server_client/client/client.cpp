#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include "../protocol/msg.pb.h"
#include "../net/protobuf_codec.h"

using boost::asio::ip::tcp;

int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    boost::asio::io_service io;
    tcp::socket sock(io);
    tcp::endpoint endpoint(
        boost::asio::ip::address::from_string("127.0.0.1"),
        8888
    );
    sock.connect(endpoint);

    msg::LoginReq req;
    req.set_account("test123");
    std::string pack = ProtobufCodec::Encode(1001, 0, req);
    boost::asio::write(sock, boost::asio::buffer(pack));

    char len_buf[4];
    boost::asio::read(sock, boost::asio::buffer(len_buf, 4));
    uint32_t body_len = ntohl(*(uint32_t*)len_buf);

    char buf[4096];
    boost::asio::read(sock, boost::asio::buffer(buf, body_len));

    msg::Packet p;
    p.ParseFromArray(buf, body_len);
    msg::LoginAck ack;
    ack.ParseFromString(p.body());

    std::cout << "login ok uid: " << ack.uid() << std::endl;

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}