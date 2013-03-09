#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

/**
 * gcc -o prime prime.c -lm
 * */

static int prime(const int number)
{
    int sqrt_v = 0;
    int ret = 0;
    int k = 0;

    assert(number >= 0);

    if (number <= 3)
    {
        ret = 1;
        goto FINISH;
    }

    sqrt_v = (int)ceil(sqrt(number));
    for (k = 2; k <= sqrt_v; k++)
    {
        /*
        printf("\n k = %d, number: %d, sqrt_v = %d, (%d %% %d) = %d\n",
            k, number, sqrt_v, number, k, (number % k));
        */
        if (0 == (number % k))
        {
            ret = 0;
            goto FINISH;
        }
    }

    ret = 1;
FINISH:
    return ret;
}

int main(int argc, char *argv[])
{
    static const int MAX_HASH_NUM = (1ULL << 31) - 1;
    int prime_number[] = {1, 2, 3, 5, 7,
                          11, 13, 17, 19, 23,
                          29, 31, 37, 0};
    int i = 0;
    int k = 0;
    int count = 0;
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
    for (i = 1; i < max_num; i++)
    {
        if (0 == prime(i))
        {
            continue;
        }

        fprintf(stdout, "%d\t", i);
        count++;
        if (0 == (count % fields_num))
        {
            fprintf(stdout, "\n");
            count = 0;
        }
    }
    fprintf(stdout, "%s---end of prime---\n", count < fields_num ? "\n" : "");

    return 0;
}

