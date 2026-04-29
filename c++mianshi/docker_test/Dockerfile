# 基础镜像：CentOS7（兼容游戏服务依赖）
FROM centos:7

# 安装C++运行依赖、网络工具（游戏服务必备）
#RUN yum install -y gcc-c++ libstdc++ net-tools telnet && \
#    yum clean all

# 工作目录
WORKDIR /game_server

# 拷贝编译好的游戏服务二进制文件
COPY ./game_server_bin .
# 拷贝配置文件
COPY ./config ./config

# 暴露游戏端口
EXPOSE 8080/tcp 27015/udp

# 容器启动命令
CMD ["./game_server_bin"]