#include "util.h"

int get_localtime_str(char *src, size_t buf_len)
{
    assert(NULL != src);
    assert(buf_len > 0);

    struct tm* p_tm = NULL;
    time_t tm = time(NULL);
    p_tm = localtime(&tm);

    snprintf(src, buf_len, "%d-%02d-%02d %02d:%02d:%02d",
        p_tm->tm_year + 1900, p_tm->tm_mon + 1, p_tm->tm_mday,
        p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);

    return 0;
}

/*
#if (DAEMON)
    if (daemonize(0, 1) == -1)
    {
        fprintf(stderr, "failed to daemon() in order to daemonize\n");
        exit(EXIT_FAILURE);
    }
#endif
*/
int daemonize(int nochdir, int noclose)
{
    int fd;

    switch (fork())
    {
    case -1:
        return (-1);
    case 0:
        break;
    default:
        _exit(EXIT_SUCCESS);
    }

    if (setsid() == -1)
        return (-1);

    if (nochdir == 0)
    {
        if(chdir("/") != 0)
        {
            perror("chdir");
            return (-1);
        }
    }

    if (noclose == 0 && (fd = open("/dev/null", O_RDWR, 0)) != -1)
    {
        if (dup2(fd, STDIN_FILENO) < 0)
        {
            perror("dup2 stdin");
            return (-1);
        }
        if (dup2(fd, STDOUT_FILENO) < 0)
        {
            perror("dup2 stdout");
            return (-1);
        }
        if (dup2(fd, STDERR_FILENO) < 0)
        {
            perror("dup2 stderr");
            return (-1);
        }

        if (fd > STDERR_FILENO)
        {
            if (close(fd) < 0)
            {
                perror("close");
                return (-1);
            }
        }
    }

    return(0);
}

static void handler(int signo)
{
    fprintf(stderr, "Get one signal: %d. man sigaction.\n", signo);
    return (void)0;
}

void signal_setup()
{
    static int signo[] = {
        SIGHUP,
        SIGINT,     /* ctrl + c */
        SIGCHLD,    /* 僵死进程或线程的信号 */
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

indext_t hash_word(const char *key, int hash_table_size)
{
    indext_t hash_value = 0;

    if (NULL == key || hash_table_size <= 0)
    {
        return 0;
    }

    while ((u_char)*key)
    {
        hash_value = (hash_value << 5) + (u_char)*key++;
    }
    logprintf("[debug] hash_value: %lu", hash_value);

    return hash_value % hash_table_size;
}
