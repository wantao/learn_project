
cmake_minimum_required(VERSION 3.16)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -Wall -Wextra")
project(GameServer)
set(CMAKE_CXX_STANDARD 11)

find_package(Boost REQUIRED COMPONENTS system thread)
find_package(Protobuf REQUIRED)

include_directories(./ ${Boost_INCLUDE_DIRS} ${PROTOBUF_INCLUDE_DIRS})

set(PROTO
    protocol/msg.pb.cc
)

set(SRC
    main.cpp
    net/asio_server.cpp
    net/asio_session.cpp
    db/mysql_pool.cpp
    db/RedisClusterClient.cpp
    async/async_db.cpp
    async/SyncDB.cpp
    shard/shard_tool.cpp
    logic/role_logic.cpp
    ${PROTO}
)

add_executable(game_server ${SRC})
add_executable(game_client client/client.cpp ${PROTO})
add_executable(game_stress stress/stress_test.cpp)

target_link_libraries(game_server
    ${Boost_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    mysqlclient
    hiredis
    hiredis_cluster
    pthread
    dl
)

target_link_libraries(game_client ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES} pthread)
target_link_libraries(game_stress ${Boost_LIBRARIES} pthread)