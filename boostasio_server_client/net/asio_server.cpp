#include "asio_server.h"
#include <boost/bind.hpp>
#include <functional>
#include <memory>  // 添加这一行

AsioServer::AsioServer(boost::asio::io_service& io, int port)
    : io_(io), acceptor_(io, tcp::endpoint(tcp::v4(), port)) {}

void AsioServer::Start() {
    Accept();
}

void AsioServer::Accept() {
    auto socket = std::make_shared<tcp::socket>(io_);
    std::cout << "new connection...socket:" << socket<< std::endl;
    acceptor_.async_accept(*socket,
        [this, socket](const boost::system::error_code& ec) {
            HandleAccept(ec, std::move(*socket));
        });
}

void AsioServer::HandleAccept(const boost::system::error_code& ec, tcp::socket socket) {
    std::cout << "HandleAccept...socket:" << " ec:" << ec << std::endl;
    if (!ec) {
        std::shared_ptr<Session> sess(new Session(std::move(socket)));
        sess->Start();
    }
    Accept();
}