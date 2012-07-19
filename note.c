1. netstata -a
2. 处理信号的函数原型
    ANSI C:
        void handler(int);

    POSIX SA_SIGINFO:
        void handler(int, siginfo_t *info, ucontext_t *uap);
3. 我们可以把某个信号的处置设定为 SIG_IGN 来忽略(ignore)它. SIGKILL 和 SIGSTOP 信号不能忽略.
4. 重启被中断的系统调用,对于 accept, read, write, select 和 open 之类的函数来说是合适的.有一个函数不能重启 connect.如果此函数返回 EINTR,不能再次调用它,否则将立即返回一个错误.当 connect 被一个捕获的信号中断而且不自动重启时,必须调用 select 来等待连接完成.
5. accept 返回前连接中止
  POSIX 指出返回的 errno 值必须是 ECONNABORTED ("software caused connection abort", 软件引起的连接中止). POSIX 作出的修改的理由在于: 流子系统(streams substystem)中发生某些致命的协议相关事件时,也会返回 EPROTO.要是对于由客户引起的一个已建立连接的非致命中止也返回同样的错误,那么服务器就不知道该不该再次调用 accept,换成 ECONNABORTED 错误,服务器就可以忽略它,再次调用 accept.
6. 服务器主机崩溃
  如果客户阻塞在 read 调用上,该调用将返回一个错误.假设服务器主机已崩溃,从而对客户的数据分节根本没有响应,那么所返回的错误是 ETIMEDOT.然而如果某个中间路由器断定服务器主机已不可达,从而响应一个 "destination unreachable"(目的地不可达)ICMP 消息,那么所返回的错误是 EHOSTUNREACH 或 ENETUNREACH.
7. 时间的结构体
struct timeval
{
    long tv_sec;    /** seconds */
    long tv_usec;   /** microseconds */
}

struct timespce
{
    time_t tv_sec;  /** seconds */
    long   tv_nsec; /** nanoseconds */
}

struct tm {
    int tm_sec;     /* seconds after the minute [0-60] */
    int tm_min;     /* minutes after the hour [0-59] */
    int tm_hour;    /* hours since midnight [0-23] */
    int tm_mday;    /* day of the month [1-31] */
    int tm_mon;     /* months since January [0-11] */
    int tm_year;    /* years since 1900 */
    int tm_wday;    /* days since Sunday [0-6] */
    int tm_yday;    /* days since January 1 [0-365] */
    int tm_isdst;   /* Daylight Savings Time flag */
    long    tm_gmtoff;  /* offset from CUT in seconds */
    char    *tm_zone;   /* timezone abbreviation */
};

typedef long time_t;        /* time value */
8. TIME_WAIT 状态
TIME_WAIT 状态有兩个存在的理由:
  1).可靠的实现 TCP 全双工连接的终止;
  2).允许老的重复分节在网络中消逝.
执行主动关闭的那端经历了 TIME_WAIT 状态.该端点停留在此状态的持续时间是最长分节生命期(maximum segment lifetime, MSL)的兩倍,有时候称之为 2MSL.
TCP 必须防止来自某个连接的老的重复分组在该连接已终止后再现,从而被误认解成属于同一个连接的某个新的化身(incarnation),为了做到这一点,TCP 将不给处于 TIME_WAIT 状态的连接发起新的化身.既然 TIME_WAIT 状态的持续时间是 MSL 的 2 倍,这就足以让某个方向上的分组最多存活 MSL 秒即被丢弃,另一个方向上的应答最多存活 MSL 秒也被丢弃.通过实施这个规则,保证每成功建立一个 TCP 连接时,来自该连接先前化身的老的重复分组都已在网络中消逝了.
9. TCP_NODELAY 套接字选项
开启本选项将禁止 TCP 的 Nagle 算法.默认情况下该算法是启动的.
Nagle 算法的目的在于减少广域网(WAN)上小分组的数目.该算法指出:如果某个给定的连接上有待确认数据(outstanding data),那么原本应该作为用户写操作之响应的该连接上立即发送相应小分组的行为就不会发生,直到现有数据被确认为止.这里'小'分组的定义就是小于 MSS 的任何分组.TCP 总是尽可能地发送最大大小的分组, Nagle 算法的目的在于防止一个连接在任何时刻有多少小分组待确认.
