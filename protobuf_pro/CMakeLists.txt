cmake_minimum_required(VERSION 3.10)
project(protobuf_demo)

# 改成 C++11（兼容老 GCC）
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate_cpp(PROTO_SRC PROTO_HDR order.proto)

add_executable(order_service 
    main.cpp 
    ${PROTO_SRC} 
    ${PROTO_HDR}
)

target_link_libraries(order_service ${PROTOBUF_LIBRARIES})