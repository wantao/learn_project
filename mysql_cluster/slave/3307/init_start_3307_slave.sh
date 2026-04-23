ps -ef | grep mysqld | grep "3307/my.cnf" | awk '{print $2}' | xargs kill -9
sleep 2
rm -rf /data/mysql/3307/data/*
rm -rf /data/mysql/3307/logs/*
cp -f 3307_my.cnf /data/mysql/3307/my.cnf
chown mysql:mysql /data/mysql/3307/my.cnf
mysqld --initialize --user=mysql --datadir=/data/mysql/3307/data
mysqld --defaults-file=/data/mysql/3307/my.cnf --user=mysql --skip-grant-tables &
sleep 2
mysql --socket=/data/mysql/3307/mysql.sock < 3307_slave_account.sql
./init_slave_db.sh
sed -i 's/^#read_only=/read_only=/; s/^#super_read_only=/super_read_only=/' /data/mysql/3307/my.cnf
ps -ef | grep mysqld | grep "3307/my.cnf" | awk '{print $2}' | xargs kill -9
sleep 2
mysqld --defaults-file=/data/mysql/3307/my.cnf --user=mysql &
sleep 5 
