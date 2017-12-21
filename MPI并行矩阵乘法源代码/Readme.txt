并行矩阵乘算法实现：
1、共有三个算法：分行算法，fenhang.c；分行且分列算法，fenhang+fenlie.c；
                Cannon算法，cannon-MS.c,cannon-peer.c,
                其中，cannon-MS.c程序为主从模式编程，数据块的迁移由进程间通信完成；cannon-peer.c程序为对等模式编程，数据块的移动由0号进程散发。
2、程序为C+MPI开发，运行环境为 Linux+MPICH3；
