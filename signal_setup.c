#include "unp.h"

#define SIGNO_END 11111

#define GETUTIME(t) ((t.tv_sec) * 1000000 + (t.tv_usec))
#define GETSTIME(t) (t.tv_sec)
#define GETMTIME(t) ((((t.tv_sec) * 1000000 + (t.tv_usec))) / 1000)

//void handler(int signo, siginfo_t *info, ucontext_t *uap)
void handler(int signo)
{
    //fprintf(stderr, "[%s %s] Get one signal: %d. man sigaction.\n", __DATE__, __TIME__, signo);
    fprintf(stderr, "Get one signal: %d. man sigaction.\n", signo);
    return (void)0;
}

void signal_setup()
{
    static int signo[] = {
        SIGHUP,
        SIGINT,     /* ctrl + c */
        SIGCHLD,
        SIGPIPE,
        SIGALRM,
        SIGUSR1,
        SIGUSR2,
        SIGTERM,
        //SIGCLD,

#ifdef  SIGTSTP
        /* background tty read */
        SIGTSTP,
#endif
#ifdef  SIGTTIN
        /* background tty read */
        SIGTTIN,
#endif
#ifdef SIGTTOU
        SIGTTOU,
#endif
        SIGNO_END
    };

    int i = 0;
    struct sigaction sa;
    //sa.sa_handler = SIG_IGN;    //设定接受到指定信号后的动作为忽略
    sa.sa_handler = handler;
    sa.sa_flags   = SA_SIGINFO;

    if (-1 == sigemptyset(&sa.sa_mask))   //初始化信号集为空
    {
        fprintf(stderr, "failed to init sa_mask.\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; SIGNO_END != signo[i]; i++)
    {
        //屏蔽信号
        if (-1 == sigaction(signo[i], &sa, NULL))
        {
            fprintf(stderr, "failed to ignore: %d\n", signo[i]);
            exit(EXIT_FAILURE);
        }
    }

    return (void)0;
}

#if (TESTING)
#define DEFAULT_PERCENT 5

int main(int argc, char *argv[])
{
    int rand_num = 0;
    int percent  = DEFAULT_PERCENT;
    struct timeval tv;

    signal_setup();

    /* parse argv */
    if (argc > 1 && argv[1][0])
    {
        percent = atoi(argv[1]);
        if (percent < DEFAULT_PERCENT)
        {
            percent = DEFAULT_PERCENT;
        }
    }

    while (1)
    {
        gettimeofday(&tv, NULL);
        srand(GETSTIME(tv));
        rand_num = rand() % 100;
        if (rand_num && rand_num <= percent)
        {
            //fprintf(stderr, "[%s %s] Good, get it: %d\n", __DATE__, __TIME__, (int)rand_num);
            fprintf(stderr, "Good, get it: %d\n", (int)rand_num);
            usleep((useconds_t)(1000000 - rand_num));
        }
    }

    return 0;
}
#endif
