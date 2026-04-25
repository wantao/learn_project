systemctl restart mysqld
echo "11"
./master_slave_account.sh
echo "222"
./slave_config.sh
echo "3333"
cd slave/3307
./init_start_3307_slave.sh
./config_3307_master_slave.sh
cd ..
cd ..
cd slave/3308
./init_start_3308_slave.sh
./config_3308_master_slave.sh
