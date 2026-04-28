pre_soft.sh:预安装软件
deadlock.cpp:模拟死锁
deadlock_fix.cpp:修复死锁
build.sh:编译
#死锁排查
    # 1. 查PID
    ps -ef | grep 进程名

    # 2. GDB附加
    gdb -p PID

    # 3. 打印所有线程栈
    thread apply all bt

    # 4. 查看锁等待
    info threads
