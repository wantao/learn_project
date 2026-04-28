线上不中断程序生成coredump
    yum install gdb -y
    gcore -o /corefile/game_server_dump 5789(pid)
线上内存增长排查
    查看整体内存概况：
        free -h
            重点关注 available 列，它显示了可用于启动新应用的真实空闲内存
            [root@localhost onlin_check_memleak]# free -h
                total        used        free      shared  buff/cache   available
                Mem:           3.7G        1.0G        1.8G         18M        943M        2.4G
                Swap:          2.0G          0B        2.0G
        
    找出内存消耗前20名大户
        ps aux --sort=-%mem | head -20
        这条命令会按内存使用率倒序排列，列出前20个进程。记下嫌疑进程的 PID。
    观察进程内存变化
        top -p <PID>
        [root@localhost ~]# top -p 5789
            top - 00:06:55 up  5:23,  4 users,  load average: 0.01, 0.02, 0.05
            Tasks:   1 total,   0 running,   1 sleeping,   0 stopped,   0 zombie
            %Cpu(s):  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
            KiB Mem :  3861288 total,  1846224 free,  1049244 used,   965820 buff/cache
            KiB Swap:  2098172 total,  2098172 free,        0 used.  2561632 avail Mem 

            PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND                                                                                                                 
            5789 root      20   0   34120  13744   1396 S   0.0  0.4   0:00.04 game_server                                                                                                             
        持续观察进程的 RES（常驻物理内存）和 VIRT（虚拟内存）两列。如果 RES 稳定增长不回落，基本可以确定是内存泄漏。
    valgrind暴露内存泄漏问题
        valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./game_server


        ==6426== 8,192 bytes in 2 blocks are definitely lost in loss record 6 of 6
        ==6426==    at 0x4C2AC38: operator new[](unsigned long) (vg_replace_malloc.c:433)
        ==6426==    by 0x4009EF: createPlayerLeak() (game_server.cpp:17)
        ==6426==    by 0x400A0E: serverLoop() (game_server.cpp:23)
        ==6426==    by 0x400A62: main (game_server.cpp:30)
        ==6426== 
        ==6426== LEAK SUMMARY:
        ==6426==    definitely lost: 8,192 bytes in 2 blocks
        ==6426==    indirectly lost: 0 bytes in 0 blocks
        ==6426==      possibly lost: 0 bytes in 0 blocks
        ==6426==    still reachable: 105 bytes in 5 blocks
        ==6426==                       of which reachable via heuristic:
        ==6426==                         stdstring          : 89 bytes in 3 blocks
        ==6426==         suppressed: 0 bytes in 0 blocks
        ==6426== 
        ==6426== For lists of detected and suppressed errors, rerun with: -s
        ==6426== ERROR SUMMARY: 4 errors from 4 contexts (suppressed: 0 from 0)
