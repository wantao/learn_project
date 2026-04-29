#!/bin/bash
# 游戏服务器运维脚本：game_server.sh

# 配置项
SERVER_BIN="./game_server"
PID_FILE="./game_server.pid"
LOG_FILE="./game_server_ctl.log"

# 日志函数
log() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" >> $LOG_FILE
}

# 检查进程是否运行
check_pid() {
    if [ -f $PID_FILE ]; then
        PID=$(cat $PID_FILE)
        if ps -p $PID > /dev/null; then
            return 0
        else
            rm -f $PID_FILE
            return 1
        fi
    fi
    return 1
}

# 启动服务
start() {
    if check_pid; then
        echo "游戏服务器已运行，PID: $(cat $PID_FILE)"
        exit 1
    fi
    echo "启动游戏服务器..."
    nohup $SERVER_BIN > /dev/null 2>&1 &
    echo $! > $PID_FILE
    log "服务启动成功，PID: $!"
    echo "启动成功，PID: $!"
}

# 停止服务
stop() {
    if ! check_pid; then
        echo "游戏服务器未运行"
        exit 1
    fi
    PID=$(cat $PID_FILE)
    kill -9 $PID
    rm -f $PID_FILE
    log "服务停止成功，原PID: $PID"
    echo "停止成功"
}

# 状态检查
status() {
    if check_pid; then
        echo "游戏服务器运行中，PID: $(cat $PID_FILE)"
    else
        echo "游戏服务器未运行"
    fi
}

# 参数判断
case "$1" in
    start) start ;;
    stop) stop ;;
    restart) stop; start ;;
    status) status ;;
    *) echo "用法: $0 {start|stop|restart|status}" ;;
esac