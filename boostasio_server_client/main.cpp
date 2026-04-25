#include <boost/asio.hpp>
#include "net/asio_server.h"
#include "async/async_db.h"
#include "db/mysql_pool.h"
#include "db/RedisClusterClient.h"

int main() {
    MySQLPool::Instance().Init("127.0.0.1", "root", "Wt315771557#", "game_user", 3306);

    // 配置集群节点
    std::vector<RedisClusterClient::NodeConfig> nodes = {
        {"127.0.0.1", 7001, "Redis@123"},
        {"127.0.0.1", 7002, "Redis@123"},
        {"127.0.0.1", 7003, "Redis@123"}
    };
    
    // 初始化集群
    if (!RedisClusterClient::Instance().init(nodes, "Redis@123")) {
        std::cerr << "Failed to initialize Redis cluster" << std::endl;
        return -1;
    }

    AsyncDB::Instance().Init();  // 修复 -> 为 .

    boost::asio::io_service io;
    AsioServer srv(io, 8888);
    srv.Start();

    io.run();
    return 0;
}