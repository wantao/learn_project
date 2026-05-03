generate_so:
    生成动态连接库so文件
hotfix_server
    通过加载so文件实现热更
    C++ 本身无原生热更，基于动态库（.so/.dll）+ 函数指针替换 + 原子操作 / 无锁保护实现
    核心技术要点
        （1）动态库加载：dlopen / dlsym（Linux）
            把核心战斗逻辑编译为独立动态库（.so），主程序不直接编译战斗逻辑
            dlopen：运行时加载新的动态库，不重启进程
            dlsym：从新库中获取更新后的函数地址
            必须用 RTLD_NOW | RTLD_GLOBAL 保证符号全局可见、立即解析
        （2）函数指针替换（核心机制）
            主程序不直接调用战斗函数，统一通过全局函数指针调用
            热更时：用新库函数地址原子替换旧指针，所有新请求立即使用新逻辑
            旧请求：继续执行旧逻辑，执行完毕后旧库可安全卸载
        （3）原子操作 / 内存屏障（保证线程安全）
            游戏服务器是多线程高并发，必须用 C++11 std::atomic 替换函数指针
            禁止指令重排，保证所有线程瞬间看到新函数地址，无中间态
            绝对不能用普通赋值，会出现线程安全问题
        （4）无侵入、无状态要求
            战斗逻辑必须无状态：不能在函数内持有全局变量、缓存、连接句柄
            热更只替换代码逻辑，不修改内存数据，保证战斗数据不丢失
            旧动态库：等待所有正在执行的旧函数退出后，dlclose 安全卸载
        （5）版本管理与回滚
            热更前备份旧.so，加载失败立即回滚到旧函数指针
            主程序预留热更钩子，支持灰度更新（只让部分玩家使用新逻辑）
stress_test
    压测
        安装wrk：
            # 1. 基础编译依赖
            sudo yum install -y gcc gcc-c++ make automake autoconf libtool openssl-devel

            # 2. 安装 wrk2 压测工具（CentOS7 专用）
            cd /usr/local/src
            git clone https://github.com/giltene/wrk2.git
            cd wrk2
            make
            cp wrk /usr/local/bin/

            # 验证
            wrk --version
        模拟热更新（不停止服务）
            # 测试接口
            curl http://127.0.0.1:8080/battle/calculate
        执行热更（加载新库）
            gdb -p $(pidof server) -ex "call load_lib(\"./libbattle_new.so\")" -ex q
        验证热更成功
            curl http://127.0.0.1:8080/battle/calculate
        压测目标
            验证热更后：
                CPU 不飙升
                延迟不增加
                吞吐不下降
                无内存泄漏
            压测工具：
                wrk
            压测参数
                接口：/battle/calculate
                线程：8
                连接：100
                RPS：5000
                时长：10 分钟
            压测命令:
                wrk -t8 -c100 -d600s -R5000 http://127.0.0.1:8080/battle/calculate
            压测结果：
                [root@localhost stress_test]# wrk -t1 -c10 -d200s -R5000 http://127.0.0.1:8080/battle1/calculate1
                Running 3m test @ http://127.0.0.1:8080/battle1/calculate1
                1 threads and 10 connections
                Thread calibration: mean lat.: 592.329ms, rate sampling interval: 2598ms
                Thread Stats   Avg      Stdev     Max   +/- Stdev
                    Latency     9.15ms   54.02ms 687.10ms   97.34%
                    Req/Sec     5.01k   163.03     5.87k    93.15%
                999888 requests in 3.33m, 81.05MB read
                Requests/sec:   4999.42
                Transfer/sec:    414.99KB
            如何看监控：
                看 CPU：top -p $(pidof server)
                看内存：watch -n1 free -h
                