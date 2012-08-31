#include "util.h"

int get_localtime_str(char *src, const size_t buf_len)
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

void signal_setup(void)
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

indext_t hash(const char *key, const int hash_table_size)
{
    indext_t hash_value = 0;

    if (NULL == key || hash_table_size <= 0)
    {
        return 0;
    }

    while ((u_char)*key)
    {
        hash_value = hash_value + (hash_value << 5) + (u_char)*key++;
    }

#if (_DEBUG_)
    logprintf("[debug] hash_value: %lu", hash_value);
#endif

    return hash_value % hash_table_size;
}

char * str_replace(char *src, const size_t buf_size, const char *search, const char *replace)
{
    char *p   = NULL;
    char *pcp = NULL;
    size_t s_len = 0;   /** stlen(search)*/

    char t_buf[TMP_STR_BUF_LEN];

    if ( NULL == src || buf_size <= 0 || NULL == replace || NULL == search)
    {
        goto FINISH;
    }

    t_buf[0] = '\0';
    s_len = strlen(search);

    pcp = src;
    p   = strstr(src, search);

    if (NULL == p)
    {
        goto FINISH;
    }

    while (NULL != p)
    {
        *p = '\0';
        strncat(t_buf, pcp, TMP_STR_BUF_LEN - strlen(t_buf) - 1);
        strncat(t_buf, replace, TMP_STR_BUF_LEN - strlen(t_buf) - 1);
        p += s_len;
        pcp = p;

        p = strstr(pcp, search);
    }

    if (pcp)
    {
        strncat(t_buf, pcp, TMP_STR_BUF_LEN - strlen(t_buf) - 1);
    }

    if (strlen(t_buf))
    {
        memmove(src, t_buf, buf_size - 1);
        src[buf_size - 1] = '\0';
    }

FINISH:
    return src;
}

char *strtolower(char *src, const size_t buf_len, const char *encoding)
{
    unsigned char cmp;
    size_t i = 0;

    if (NULL == src || NULL == encoding)
    {
        return NULL;
    }

    for (i = 0; i < buf_len && src[i]; )
    {
        cmp = (unsigned char)src[i];
        if (0 == strncmp(encoding, "utf-8", 5))
        {
            if (cmp == 9 || cmp == 10 || (32 <= cmp && cmp <= 126))
            {
                src[i] = tolower(src[i]);
                i++;
            }
            else if (194 <= cmp && cmp <= 223)
            {
                i += 2;
            }
            else if (224 <= cmp && cmp <= 239)
            {
                i += 3;
            }
            else if (240 <= cmp && cmp <= 247)
            {
                i += 4;
            }
            else if (248 <= cmp && cmp <= 251)
            {
                i += 5;
            }
            else if (252 == cmp || 253 == cmp)
            {
                i += 6;
            }
            else
            {
                i++;
            }
        }
        else
        {
            /** gbk */
            if (cmp > 127)
            {
                i++;
            }
            else
            {
                src[i] = tolower(src[i]);
            }

            i++;
        }
    }

    return src;
}

int cut_str(const char *src,
            char *des,
            const size_t des_buf_len,
            const char *charset,
            unsigned int length,    /** 宽字符串的字数 */
            const char *suffix)
{
    int ret = 0;
    unsigned int suffix_len = strlen(suffix);
    unsigned int str_len    = strlen(src);

    unsigned int wchar_count = 0;
    unsigned int cut_count = 0;
    unsigned char t = 0;

    int tn = 0;

    if (NULL == src || NULL == des || length <= 0 || str_len >= des_buf_len)
    {
        des[0] = '\0';
        ret = -11;
        goto FINISH;
    }

    if (str_len <= length && str_len < des_buf_len)
    {
        memmove(des, src, str_len);
        des[des_buf_len - 1] = '\0';

        goto FINISH;
    }

    length -= suffix_len;
    if (0 == strncmp(charset, "utf-8", 5))
    {
        cut_count = 0;
        //logprintf("src str: [%s]", src);
        while (wchar_count < length && cut_count < des_buf_len - suffix_len 
                && cut_count < str_len)
        {
            t = (unsigned char)src[cut_count];
            if (t == 9 || t == 10 || (32 <= t && t <= 126))
            {
                tn = 1;
            }
            else if (194 <= t && t <= 223)
            {
                tn = 2;
            }
            else if (224 <= t && t <= 239)
            {
                tn = 3;
            }
            else if (240 <= t && t <= 247)
            {
                tn = 4;
            }
            else if (248 <= t && t <= 251)
            {
                tn = 5;
            }
            else if (t == 252 || t == 253)
            {
                tn = 6;
            } else
            {
                tn = 1;
            }

            wchar_count++;
            cut_count += tn;
        }

        //logprintf("cut_count: %d", cut_count);
        for (tn = 0; tn < cut_count; tn++)
        {
            des[tn] = src[tn];
        }
        //logprintf("after des: %s", des);
    }
    else
    {
        for (tn = 0; tn < length - 1; tn++)
        {
            des[tn] = src[tn];
            t = (unsigned char)src[tn];
            if (t  > 127)
            {
                tn++;
                des[tn] = src[tn];
            }
        }

    }
    ret = cut_count;

    strncat(des, suffix, des_buf_len - strlen(des) - 1);
    des[des_buf_len - 1] = '\0';

FINISH:
    return ret;
}

static unsigned char to_hex(unsigned char code)
{
    static unsigned char hex[] = "0123456789ABCDEF";
    return hex[code & 15];
}

int url_encode(char *str, int ext)
{
    int ret = 0;
    unsigned char *pstr = (unsigned char *)str;
    unsigned char  buf[DEFAULT_LINK_LENGTH] = {0};
    unsigned char *pbuf = buf;

    if (NULL == str)
    {
        ret = -1;
        goto FINISH;
    }

    while (*pstr && (pbuf - buf + 4) < DEFAULT_LINK_LENGTH)
    {
        /* Allow only alphanumeric chars and '_', '-', '.'; escape the rest */
        /** Come from php source code. */
        if (isalnum(*pstr) || NULL != strchr("_-.~", *pstr)
            || (ext && NULL != strchr("%?&=/:", *pstr)))
        {
            *pbuf++ = *pstr;
        }
        else if (*pstr == ' ')
        {
            *pbuf++ = '+';
        }
        else
        {
            *pbuf++ = '%';
            *pbuf++ = to_hex(*pstr >> 4);
            *pbuf++ = to_hex(*pstr & 15);
        }

        pstr++;
    }
    *pbuf = '\0';

    snprintf(str, DEFAULT_LINK_LENGTH, "%s", buf);

FINISH:
    return ret;
}

int prefix_cmp(const char *prefix, const char *cmp_str)
{
    assert(NULL != prefix);
    assert(NULL != cmp_str);

    int ret = 0;
    int i = 0;
    size_t prefix_len = strlen(prefix);
    size_t cmp_str_len= strlen(cmp_str);
    size_t count = cmp_str_len;
    size_t match = 0;

    if (cmp_str_len > prefix_len)
    {
        goto FINISH;
    }

    for (i = 0; i < count; i++)
    {
        if ((u_char)prefix[i] == (u_char)cmp_str[i])
        {
            match++;
        }
    }

    if (match && match == count)
    {
        ret = 1;
    }

FINISH:
    return ret;
}
