Protobuf Demo 项目说明
    该项目是一个基于 Protocol Buffers（Protobuf）的简单演示程序，运行于 CentOS 7 系统，展示了如何在 C++ 项目中集成 Protobuf 进行数据序列化与反序列化，以订单（order）相关数据处理为例，实现了基础的 Protobuf 消息定义、编译与调用流程。
一、项目功能
    定义订单（Order）相关的 Protobuf 数据结构（order.proto）；
    通过 CMake 构建工程，自动编译 Protobuf 文件生成 C++ 代码；
    演示 Protobuf 消息的序列化、反序列化及基础数据操作。
二、文件说明
    表格
    文件名	作用说明
    CMakeLists.txt	项目构建配置文件，指定编译规则、依赖库（Protobuf）、编译选项及可执行程序生成逻辑
    order.proto	Protobuf 消息定义文件，定义订单相关的数据结构（如订单号、金额、商品列表等）
    main.cpp	主程序文件，调用 Protobuf 生成的代码，实现消息的创建、序列化、反序列化等逻辑
    PROTO_SRC/PROTO_HDR	编译 order.proto 自动生成的 C++ 源文件和头文件（构建后生成，不在源码库）
三、前置依赖安装（CentOS 7）
    sudo yum install protobuf-devel cmake gcc-c++ -y 如果ok，无需执行下面步骤
    1. 基础编译环境
    bash
    运行
    # 安装 gcc、g++、cmake 等基础工具
    yum install -y gcc gcc-c++ cmake make
    # 确认版本（cmake 需 ≥3.10，CentOS 7 自带 cmake 版本较低，需升级）
    cmake --version
    2. 升级 CMake（若版本 < 3.10）
    CentOS 7 官方源的 CMake 版本较低，需手动升级：
    bash
    运行
    # 安装依赖
    yum install -y wget openssl-devel
    # 下载并解压 CMake 3.20（LTS 版本）
    wget https://cmake.org/files/v3.20/cmake-3.20.6-linux-x86_64.tar.gz
    tar -zxvf cmake-3.20.6-linux-x86_64.tar.gz
    # 替换系统默认 cmake
    cp -rf cmake-3.20.6-linux-x86_64/bin/* /usr/bin/
    cp -rf cmake-3.20.6-linux-x86_64/lib/* /usr/lib64/
    # 验证版本
    cmake --version  # 应显示 3.20.x
    3. 安装 Protobuf 库及工具
    bash
    运行
    # 安装 Protobuf 编译依赖
    yum install -y autoconf automake libtool
    # 下载 Protobuf 源码（推荐 3.x 稳定版）
    git clone https://github.com/protocolbuffers/protobuf.git
    cd protobuf
    git checkout v3.20.3  # 选择稳定版本
    # 编译安装
    ./autogen.sh
    ./configure --prefix=/usr/local
    make -j4  # 多核编译加速
    make install
    # 刷新库缓存
    ldconfig
    # 验证安装
    protoc --version  # 应显示 libprotoc 3.20.x
四、编译步骤
    将项目文件（CMakeLists.txt、order.proto、main.cpp）放入同一目录，进入该目录：
    bash
    运行
    cd /path/to/protobuf_demo
    创建构建目录并编译：
    bash
    运行
    mkdir build && cd build
    cmake ..  # 生成 Makefile
    make      # 编译生成可执行程序
    编译成功后，build 目录下会生成：
    自动编译 order.proto 生成的 order.pb.cc（PROTO_SRC）和 order.pb.h（PROTO_HDR）；
    可执行程序 order_service。
五、运行方法
    在 build 目录下直接执行可执行程序：
    bash
    运行
    ./order_service
    若运行成功，会输出 Protobuf 消息的序列化 / 反序列化结果（具体输出取决于 main.cpp 中的业务逻辑）；
    若提示库找不到，可手动指定库路径：
    bash
    运行
    LD_LIBRARY_PATH=/usr/local/lib ./order_service
六、常见问题
    cmake 版本过低：按「前置依赖」中步骤升级 CMake；
    protoc: command not found：确认 Protobuf 安装后 protoc 已加入系统 PATH，或手动指定 /usr/local/bin/protoc；
    链接时找不到 protobuf 库：编译时确保 find_package(Protobuf REQUIRED) 能找到库，或在 CMakeLists.txt 中手动指定 PROTOBUF_LIBRARY=/usr/local/lib/libprotobuf.so；
    CentOS 7 编译报错「C++11 特性不支持」：CMakeLists.txt 已指定 CMAKE_CXX_STANDARD 11，确保 gcc 版本 ≥4.8（CentOS 7 自带 gcc 4.8.5 满足要求）。