一键配置redis集群，三主三从
#redis.conf
    redis.conf集群配置
#预先安装
    yum install -y ruby rubygems
#config_cluster.sh
    ./config_cluster.sh:自动配置三主三从的redis集群
#运行过的环境
    [root@localhost redis_cluster]# redis-server -v
        Redis server v=3.2.12 sha=00000000:0 malloc=jemalloc-3.6.0 bits=64 build=7897e7d0e13773f
    centos7