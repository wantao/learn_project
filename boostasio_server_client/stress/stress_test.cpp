#include <boost/asio.hpp>
#include <thread>
#include <atomic>
#include <iostream>
#include <unistd.h>

using boost::asio::ip::tcp;

std::atomic<int> cnt(0);

void Client() {
    try {
        boost::asio::io_service io;
        tcp::socket sock(io);
        tcp::endpoint endpoint(
            boost::asio::ip::address::from_string("127.0.0.1"),
            8888
        );
        sock.connect(endpoint);
        cnt++;
        while (true) {
            sleep(1);
        }
    } catch (...) {}
}

int main() {
    for (int i = 0; i < 100; ++i) {
        std::thread(Client).detach();
    }

    while (true) {
        std::cout << "online: " << cnt << std::endl;
        sleep(1);
    }
    return 0;
}