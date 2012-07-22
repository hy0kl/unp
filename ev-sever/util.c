#include "ev_sever.h"

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
