pkill mysqld
sleep 2
mysqld --defaults-file=/etc/my.cnf --user=mysql &
sleep 3
mysqld --defaults-file=/data/mysql/3307/my.cnf --user=mysql &
sleep 3
mysqld --defaults-file=/data/mysql/3308/my.cnf --user=mysql &
sleep 2
mysql -uroot -p'Wt315771557#' -P3306 -e "show variables like 'server_id';"
mysql --socket=/data/mysql/3307/mysql.sock -uroot -p'Wt315771557#' -P3307 -e "show variables like 'server_id';"
mysql --socket=/data/mysql/3308/mysql.sock -uroot -p'Wt315771557#' -P3308 -e "show variables like 'server_id';"
