#include "ev_sever.h"
#include <event.h>

/**
 * gcc event-sever.c -levent
 * */

struct event    ev;
struct timeval  tv;

void time_cd(int fd, short event, void *argc)
{
    struct tm* p_tm = NULL;
    time_t tm = time(NULL);
    p_tm = localtime(&tm);

    printf("timer wakeup at: %d-%02d-%02d %02d:%02d:%02d\n",
        p_tm->tm_year + 1900, p_tm->tm_mon + 1, p_tm->tm_mday,
        p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
    event_add(&ev, &tv);    // reschedule timer

    return;
}

int
main(int argc, char *argv[])
{
    struct event_base *base = event_init();

    tv.tv_sec  = 5;
    tv.tv_usec = 0;

    evtimer_set(&ev, time_cd, NULL);
    event_add(&ev, &tv);
    event_base_dispatch(base);

    return 0;
}
