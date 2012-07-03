#include "unp.h"

int main(int argc, char *argv[])
{
    union {
        short s;
        char  c[sizeof(short)];
    } un;

    un.s = 0x0102;
    //printf("%s: ", CPU_VENDOR_OS);
    if (2 == sizeof(short))
    {
        if (1 == un.c[0] && 2 == un.c[1])
        {
            printf("big-endian\n");
        }
        else if (2 == un.c[0] && 1 == un.c[1])
        {
            printf("little-endian\n");
        }
        else
        {
            printf("unknow\n");
        }
    }
    else
    {
        printf("sizeof(short) = %d\n", (int)sizeof(short));
    }

    return 0;
}
