/**
 * @describe:
 * @author: hy0kle@gmail.com
 * */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <err.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define HAVE_IPV6 0
#define HAVE_INET_PTON  0
#define HOSTNAME_BUF_LEN    64

/* {{{ php_gethostbyaddr */
int php_gethostbyaddr(char *ip, char *host, size_t host_len)
{
#if HAVE_IPV6 && HAVE_INET_PTON
	struct in6_addr addr6;
#endif
	struct in_addr addr;
	struct hostent *hp;
    int ip_len = strlen(ip);

#if HAVE_IPV6 && HAVE_INET_PTON
	if (inet_pton(AF_INET6, ip, &addr6)) {
		hp = gethostbyaddr((char *) &addr6, sizeof(addr6), AF_INET6);
	} else if (inet_pton(AF_INET, ip, &addr)) {
		hp = gethostbyaddr((char *) &addr, sizeof(addr), AF_INET);
	} else {
		return NULL;
	}
#else
	addr.s_addr = inet_addr(ip);

	if (addr.s_addr == -1) {
		return -1;
	}

	hp = gethostbyaddr((char *) &addr, sizeof(addr), AF_INET);
#endif

	if (!hp || hp->h_name == NULL || hp->h_name[0] == '\0') {
		snprintf(host, host_len, "%s", ip);
        return 0;
	}

    snprintf(host, host_len, "%s", hp->h_name);

    return 0;
}
/* }}} */

int main(int argc, char *argv[])
{
    char hostname[HOSTNAME_BUF_LEN] =  "\0";

    if (! (argc > 1))
    {
        printf("Need more args...\n"); 
        exit(-1);
    }

    int ret = php_gethostbyaddr(argv[1], hostname, sizeof(hostname));
    if (0 != ret)
    {
        printf("call php_gethostbyaddr() error, ip: %s\n", argv[1]);
        exit(-2);
    }

    printf("ip: %s, Hostname: %s\n", argv[1], hostname);

    return 0;
}

