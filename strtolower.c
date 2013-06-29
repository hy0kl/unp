#include "unp.h"

#define STR_BUF_LEN 1024

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
int
main(int argc, char *argv[])
{
    char str[STR_BUF_LEN] = {"春天在哪里呀, lalala, It is just ABC."};

    if (argc > 1 && argv[1][0])
    {
        snprintf(str, STR_BUF_LEN, "%s", argv[1]);
    }

    printf("original str:     [%s]\n", str);
    strtolower(str, STR_BUF_LEN, "utf-8");
    printf("lowercase str is: [%s]\n", str);

    return 0;
}
