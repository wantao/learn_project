#!/bin/bash

# 1. 从主库(3306)自动获取 binlog 文件和位置
MASTER_FILE=$(mysql -uroot -p'Wt315771557#' -se "show master status\G" | grep File | awk '{print $2}')
MASTER_POS=$(mysql -uroot -p'Wt315771557#' -se "show master status\G" | grep Position | awk '{print $2}')

echo "【主库信息】"
echo "File: $MASTER_FILE"
echo "Position: $MASTER_POS"
echo "------------------------"

# 2. 生成主从配置SQL
cat > master_slave.sql <<SQL
STOP SLAVE;
RESET SLAVE ALL;

CHANGE MASTER TO
MASTER_HOST='127.0.0.1',
MASTER_PORT=3306,
MASTER_USER='repl',
MASTER_PASSWORD='Repl@123456',
MASTER_LOG_FILE='${MASTER_FILE}',
MASTER_LOG_POS=${MASTER_POS};

START SLAVE;
SQL

