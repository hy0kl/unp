#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[])
{
    static const int MAX_HASH_NUM = (1ULL << 31) - 1;  /**< 最大Hash桶大小 */
    int prime_number[] = {1, 2, 3, 5, 7, 9,
                          11, 13, 17, 19, 23,
                          29, 31, 0};
    int i = 0;
    int k = 0;
    int count = 0;
    int sqrt_v = 0;
    int fields_num = 15;
    int t_opt = 0;
    int max_num = 1000;

    fprintf(stderr, "1ULL = %llu\n", 1ULL);
    fprintf(stderr, "MAX_HASH_NUM = %d\n", MAX_HASH_NUM);

    for (i = 0; prime_number[i]; i++)
    {
        fprintf(stderr, "(1ULL << %d) - 1 = %llu\n",
            prime_number[i], (1ULL << prime_number[i]) - 1);
    }

    if (argc > 1 && argv[1][0])
    {
        t_opt = atoi(argv[1]);
        if (t_opt > 0)
        {
            max_num = t_opt;
        }
    }

    if (argc > 2 && argv[2][0])
    {
        t_opt = atoi(argv[2]);
        if (t_opt > 5)
        {
            fields_num = t_opt;
        }
    }

    fprintf(stdout, "---prime number table---\n");
    for (i = 0; i < max_num; i++)
    {
        sqrt_v = sqrt(i);
        for (k = 2; k < sqrt_v; k++)
        {
            if (0 == (i % k))
            {
                goto NEXT_NUM;
            }
        }

        fprintf(stdout, "%d\t", i);
        count++;
        if (0 == (count % fields_num))
        {
            fprintf(stdout, "\n");
            count = 0;
        }
NEXT_NUM:
        ;
    }
    fprintf(stdout, "%s---end of prime---\n", count < fields_num ? "\n" : "");

    return 0;
}
