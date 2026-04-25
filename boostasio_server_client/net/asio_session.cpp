#include "asio_session.h"
#include "../logic/role_logic.h"
#include <arpa/inet.h> // 增加ntohl/htonl头文件

Session::Session(tcp::socket socket) : socket_(std::move(socket)), uid_(0) {}

void Session::Start() { ReadLength(); }

void Session::HandleSend(const boost::system::error_code&, size_t) {}


void Session::ReadLength() {
    std::cout << "Reading length..." << std::endl;
    std::shared_ptr<Session> self(shared_from_this());
    // 修复1：用lambda回调，异步读取完成后再解析长度
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_buf_, 4),
        [self](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            if (ec) {
                std::cout << "ReadLength error: " << ec.message() << " (bytes: " << bytes_transferred << ")" << std::endl;
                return;
            }
            // 修复2：读取完成后解析长度，并用ntohl转换网络字节序→主机序
            uint32_t body_len = ntohl(*(uint32_t*)self->read_buf_);
            std::cout << "ReadLength success, body len: " << body_len << std::endl;
            
            // 安全校验：包体长度不能超过缓冲区上限（防止恶意攻击）
            if (body_len > 4096) { // 假设最大包体4096字节，可根据业务调整
                std::cout << "ReadLength error: body len too large (" << body_len << ")" << std::endl;
                return;
            }
            
            self->ReadBody(body_len);
        });
}

void Session::ReadBody(uint32_t len) {
    std::cout << "Reading ReadBody... len: " << len << std::endl;
    std::shared_ptr<Session> self(shared_from_this());
    // 修复3：lambda回调，读取包体后再调用OnPacket
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_buf_, len),
        [self, len](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            if (ec) {
                std::cout << "ReadBody error: " << ec.message() << " (bytes: " << bytes_transferred << ")" << std::endl;
                return;
            }
            self->OnPacket(self->read_buf_, len);
        });
}


void Session::OnPacket(const char* data, uint32_t len) {
    std::cout << "Reading OnPacket..." << std::endl;
    msg::Packet p;
    if (!p.ParseFromArray(data, len)) {
        std::cout<<"OnPacket parse error"<<std::endl;
        return;
    }
    uid_ = p.uid();
    std::cout << " uid_:"<<uid_<<std::endl;
    Dispatch(p.cmd(), p);
    ReadLength();
}

void Session::Dispatch(uint32_t cmd, const msg::Packet& p) {
    std::cout << " cmd:"<< cmd <<std::endl;
    switch (cmd) {
        case 1001: {
            msg::LoginReq req;
            req.ParseFromString(p.body());
            RoleLogic::Instance().HandleLogin(req, shared_from_this());
            break;
        }
        case 1003: {
            RoleLogic::Instance().GetRoleInfo(uid_, shared_from_this());
            break;
        }
        case 1005: {
            msg::HeartBeat beat;
            Send(1005, beat);
            break;
        }
    }
}