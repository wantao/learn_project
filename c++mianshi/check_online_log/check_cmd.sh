# 1. 实时查看最新日志（定位即时故障）
tail -f game_server.log

# 2. 搜索错误关键字（游戏服务常用：ERROR/崩溃/超时/连接失败）
grep -n "ERROR" game_server.log
grep -n "Segmentation fault" game_server.log  # C++进程崩溃
grep -n "timeout" game_server.log             # 网络超时（玩家掉线）

# 3. 查看报错上下文（前10行+后10行，定位完整故障场景）
grep -A 10 -B 10 "Segmentation fault" game_server.log

# 4. 统计报错次数（定位高频问题）
grep "ERROR" game_server.log | wc -l

# 5. 按时间筛选日志（定位指定时间段故障）
sed -n '/2025-05-20 10:00/,/2025-05-20 10:10/p' game_server.log

# 6. 大日志文件快速翻页查看
less game_server.log