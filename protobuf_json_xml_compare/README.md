安装protobuf,json,xml库
    yum install -y gcc gcc-c++ jsoncpp-devel libxml2-devel protobuf-devel
user.proto
    测试用protobuf文件
benchmark.cpp
    性能压力测试cpp
build.sh
    编译方法
result.png
    百万次性能测试效果图
    体积
        protobuf比json小3倍，比xml小5倍
    序列化时间
        protobuf比json快16倍数，比xml快7倍
    反序列化时间
        protobuf比json快10倍数，比xml快30倍