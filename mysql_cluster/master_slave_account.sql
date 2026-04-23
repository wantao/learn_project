# 创建主从复制专用账号
DROP USER IF EXISTS 'repl'@'%';
CREATE USER 'repl'@'%' IDENTIFIED BY 'Repl@123456';
ALTER USER 'repl'@'%' IDENTIFIED WITH mysql_native_password BY 'Repl@123456';
GRANT REPLICATION SLAVE ON *.* TO 'repl'@'%';
FLUSH PRIVILEGES;

# 查看主库状态（记录File和Position，从库配置需要）
SHOW MASTER STATUS;
