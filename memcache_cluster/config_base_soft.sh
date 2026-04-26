yum install -y libevent libevent-devel memcached
yum install -y libmemcached libmemcached-devel
memcached -u memcached -p 11211 -m 1024 -c 65535 -t 1 -d -l 0.0.0.0
memcached -u memcached -p 11212 -m 1024 -c 65535 -t 1 -d -l 0.0.0.0
memcached -u memcached -p 11213 -m 1024 -c 65535 -t 1 -d -l 0.0.0.0
memcached -u memcached -p 11214 -m 1024 -c 65535 -t 1 -d -l 0.0.0.0
ps -ef | grep memcached
netstat -tulpn | grep memcached
firewall-cmd --add-port=11211-11214/tcp --permanent
firewall-cmd --reload
memcached-tool 192.168.72.131:11211 stats

