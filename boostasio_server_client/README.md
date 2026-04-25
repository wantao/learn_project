部署测试在centos7上
高并发服务器架构
    1主2从mysql集群+3主3从redis集群
编译前期
    ./config_base_soft.sh
    ./setCore.sh
编译
    cd build
    cmake ..
    make -j4
运行测试
    ./game_server
    ./game_client