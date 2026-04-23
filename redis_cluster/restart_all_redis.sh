for port in {7001,7002,7003,7004,7005,7006};do
  redis-server /data/redis/$port/redis.conf
done

# 检查进程
ps -ef | grep redis
