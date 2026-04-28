性能抖动定位流程
    先安装：yum install -y gcc gcc-c++ gdb perf htop
    查看整体负载：htop
        你会看到：
            CPU 100%
            无 IO 等待
            无内存暴涨
            → 判断是 CPU / 算法问题
    采样:
        perf top -p $(pidof game_server)
            你会看到
                 80.0%  game_server  [.] CalculateDamage
    抓取函数调用栈
        perf record -g -p $(pidof game_server)
            
            Samples: 44K of event 'cpu-clock', 4000 Hz, Event count (approx.): 6245976895 lost: 0/0 drop: 0/0                                                                                            
            Overhead  Shared Obje  Symbol                                                                                                                                                                
            54.33%  game_server  [.] CalculateDamage                                                                                                                                                   
            45.66%  game_server  [.] std::vector<int, std::allocator<int> >::size                                                                                                                      
            0.00%  [kernel]     [k] __do_softirq                                                                                                                                                      
        等待 3 秒按 Ctrl+C
            perf report
              CalculateDamage
              GameFrame
              main                                