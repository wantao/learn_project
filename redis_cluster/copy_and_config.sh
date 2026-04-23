cp -f redis.conf /data/redis/7001/redis.conf
for port in {7002,7003,7004,7005,7006};do
  cp /data/redis/7001/redis.conf /data/redis/$port/
  sed -i "s/7001/$port/g" /data/redis/$port/redis.conf
done
