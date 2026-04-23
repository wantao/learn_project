chmod +x auto_generate_config_slave.sh
./auto_generate_config_slave.sh
mysql --socket=/data/mysql/3307/mysql.sock -uroot -p'Wt315771557#' -P3307 < master_slave.sql
sleep 2
mysql --socket=/data/mysql/3307/mysql.sock -uroot -p'Wt315771557#' -P3307 -e "show slave status\G;"
