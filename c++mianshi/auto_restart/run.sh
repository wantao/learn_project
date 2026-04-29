# 赋予执行权限
chmod +x game_server.sh

# 测试命令
./game_server.sh start    # 启动服务
#./game_server.sh status   # 查看运行状态
#./game_server.sh restart  # 重启服务
#./game_server.sh stop     # 停止服务

# 验证结果
# 1. 启动后生成 pid 文件，状态查询正常
# 2. 日志文件记录所有操作
# 3. 重复启动会提示已运行