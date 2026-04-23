mysqldump -uroot -p'Wt315771557#' --databases test  --master-data=2 --single-transaction > backup.sql
mysql --socket=/data/mysql/3308/mysql.sock -uroot -p'Wt315771557#' -P3308 < backup.sql
