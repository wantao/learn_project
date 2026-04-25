#pragma once
#include <boost/asio.hpp>
#include "asio_session.h"

using boost::asio::ip::tcp;

class AsioServer {
public:
    AsioServer(boost::asio::io_service& io, int port);
    void Start();

private:
    void Accept();
    void HandleAccept(const boost::system::error_code& ec, tcp::socket socket);

    boost::asio::io_service& io_;
    tcp::acceptor acceptor_;
};