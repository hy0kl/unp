#include "util.h"

int main(int argc, char *argv[])
{
    char  query[QUERY_LEN] = {0};
    indext_t hash_value = 0;
    int hash_table_size = 0;

    if (argc < 3)
    {
        fprintf(stderr, "need more args.\n"
                "usage: ./hash <query> <hash table size>\n");
        exit(EXIT_SUCCESS);
    }

    snprintf(query, QUERY_LEN, "%s", argv[1][0] ? argv[1] : "");
    if (argv[2][0])
    {
        hash_table_size = atoi(argv[2]);
    }

    if (!strlen(query) || hash_table_size <= 0)
    {
        fprintf(stdout, "%d", 0);
        exit(EXIT_SUCCESS);
    }

    hash_value = hash(query, hash_table_size);
    fprintf(stdout, "%lu", hash_value);

    return 0;
}
