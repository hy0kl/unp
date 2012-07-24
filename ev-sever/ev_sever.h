#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <err.h>

#include <event.h>
#include <evhttp.h>

#define SIGNO_END 11111

#define GETUTIME(t) ((t.tv_sec) * 1000000 + (t.tv_usec))
#define GETSTIME(t) (t.tv_sec)
#define GETMTIME(t) ((((t.tv_sec) * 1000000 + (t.tv_usec))) / 1000)

#define HOST_NAME_LEN   32
#define FILE_NAME_LEN   128

typedef struct _config_t
{
    int do_daemonize;
    //char *config_file;
    int  port;
    int  timeout;
    int  log_level;
    char hostname[HOST_NAME_LEN];
    char inverted_index[FILE_NAME_LEN];
    char index_dict[FILE_NAME_LEN];
} config_t;


/**
 * global config
 * */
extern config_t gconfig;

