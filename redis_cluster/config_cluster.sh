pkill redis-server
sleep 3
cp -f redis-trib.rb /usr/local/bin/redis-trib.rb
rm -rf /data/redis/700*/*
./copy_and_config.sh 
./restart_all_redis.sh
redis-cli -c -p 7001 -a Redis@123 cluster meet 127.0.0.1 7002
redis-cli -c -p 7001 -a Redis@123 cluster meet 127.0.0.1 7003
redis-cli -c -p 7001 -a Redis@123 cluster meet 127.0.0.1 7004
redis-cli -c -p 7001 -a Redis@123 cluster meet 127.0.0.1 7005
redis-cli -c -p 7001 -a Redis@123 cluster meet 127.0.0.1 7006
sleep 5
redis-cli -c -p 7001 -a Redis@123 cluster addslots {0..5461}
redis-cli -c -p 7002 -a Redis@123 cluster addslots {5462..10922}
redis-cli -c -p 7003 -a Redis@123 cluster addslots {10923..16383}
redis-cli -c -p 7001 -a Redis@123 cluster nodes
redis-cli -c -p 7004 -a Redis@123 cluster replicate $(redis-cli -c -p 7001 -a Redis@123 cluster nodes | grep '127.0.0.1:7001' | awk '{print $1}')
redis-cli -c -p 7005 -a Redis@123 cluster replicate $(redis-cli -c -p 7001 -a Redis@123 cluster nodes | grep '127.0.0.1:7002' | awk '{print $1}')
redis-cli -c -p 7006 -a Redis@123 cluster replicate $(redis-cli -c -p 7001 -a Redis@123 cluster nodes | grep '127.0.0.1:7003' | awk '{print $1}')
sleep 3
redis-cli -c -p 7001 -a Redis@123 cluster nodes
sleep 3
redis-cli -c -p 7001 -a Redis@123 cluster info
redis-cli -c -p 7001 -a Redis@123 set test 123
sleep 1
redis-cli -c -p 7001 -a Redis@123 get test
