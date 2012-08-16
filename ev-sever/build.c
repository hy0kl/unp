#include "build.h"

/**
 * global variables
 * */
config_t      gconfig;

hash_list_ext_t *hash_table  = NULL;
brief_info_t    *brief_table = NULL;
output_fp_t     output_fp;

char g_original_file[FILE_NAME_LEN] = DEFAULT_ORIGINAL_FILE;
int  g_parse_completed = 0;
/** END for global vars */

static void init_config(void)
{
    gconfig.log_level    = 4;
    gconfig.max_hash_table_size = MAX_HASH_TABLE_SIZE;
    gconfig.max_dict_table_size = MAX_DICT_TABLE_SIZE;
    snprintf(gconfig.hostname, HOST_NAME_LEN, "%s", DEFAULT_LISTEN);
    snprintf(gconfig.inverted_index, FILE_NAME_LEN, "%s", DEFAULT_INVERTED_INDEX);
    snprintf(gconfig.index_dict, FILE_NAME_LEN, "%s", DEFAULT_INDEX_DICT);

    return;
}

static int init_hash_table(void)
{
    int ret = 0;
    int i   = 0;
    size_t size = 0;

    /** 申请倒排表的内存空间 */
    size = sizeof(hash_list_ext_t) * gconfig.max_hash_table_size;
    hash_table = (hash_list_ext_t *)malloc(size);
    if (NULL == hash_table)
    {
        fprintf(stderr, "Can NOT malloc memory for hash table, need size: %ld\n",
            size);
        ret = -1;
        goto FINISH;
    }

    for (i = 0; i < gconfig.max_hash_table_size; i++)
    {
        hash_table[i].next = NULL;

        size = sizeof(char) * QUERY_LEN;
        hash_table[i].prefix = (char *)malloc(size);
        if (NULL == hash_table[i].prefix)
        {
            fprintf(stderr, "Can NOT malloc memory for index_hash_table[%d].prefix, need size: %ld\n",
                i, size);
            ret = -1;
            goto FINISH;
        }
        hash_table[i].prefix[0] = '\0';

        size = sizeof(weight_array_t);
        hash_table[i].weight_array = (weight_array_t *)malloc(size);
        if (NULL == hash_table[i].weight_array)
        {
            fprintf(stderr, "Can NOT malloc memory for index_hash_table[%d].weight_array, need size: %ld\n",
                i, size);
            ret = -1;
            goto FINISH;
        }
        hash_table[i].weight_array->count = 0;
    }

FINISH:
    return ret;
}

/**
static int init_brief_table(void)
{
    int ret = 0;
    size_t i = 0;
    size_t size = sizeof(brief_info_t) * gconfig.max_dict_table_size;

    brief_table = (brief_info_t *)malloc(size);
    if (NULL == brief_table)
    {
        fprintf(stderr, "Can NOT malloc memory for brief_table, need size: %ld\n", size);
        ret = -1;
        goto FINISH;
    }
    for (i = 0; i < gconfig.max_dict_table_size; i++)
    {
        brief_table[0].brief = NULL;
    }

FINISH:
    return ret;
}
*/

static void usage(void)
{
    printf(BUILD_PACKAGE " " BUILD_VERSION "\n");
    printf("-s <num>      max hash table size(default: %d)\n", MAX_HASH_TABLE_SIZE);
    printf("-a <num>      max dict table size(default: %d)\n", MAX_DICT_TABLE_SIZE);
    printf("-v            show version and help\n"
           "-l <level>    set log lever\n"
           "-o <file>     set original file name adn path, ABS path is better\n"
           "-i <file>     set inverted index file name and path, ABS path is better\n"
           "-x <file>     set index dict file name and path, ABS path is better\n"
           "-h            show this help and exit\n");

    return;
}

static int weight_cmp(const void *a, const void *b)
{
    weight_item_t *aa = (weight_item_t *)a;
    weight_item_t *bb = (weight_item_t *)b;

    return bb->weight - aa->weight;
}

static int parse_task(void)
{
    int ret = 0;
    char line_buf[ORIGINAL_LINE_LEN];
    char tmp_buf[ORIGINAL_LINE_LEN];
    char prefix[PREFIX_LEN];
    char log_buf[LOG_BUF_LEN];
    char *p = NULL;
    char *find = NULL;

    FILE *fp = NULL;
    int   i  = 0;
    int   k  = 0;
    int result = 0;
    int chinese_char_flag = 0;
    int hash_exist = 0;
    float weight = 0.0;
    size_t size  = 0;
    size_t str_len = 0;

    hash_list_ext_t *hash_item = NULL;
    hash_list_ext_t *tmp_hash_item = NULL;

    indext_t dict_id   = 0;
    indext_t hash_key  = 0;
    prefix_array_t prefix_array;

    fp = fopen(g_original_file, "r");
    if (! fp)
    {
        fprintf(stderr, "Can open original file: [%s]\n", g_original_file);
        ret = -1;
        goto FINISH;
    }

    while (! feof(fp))
    {
        if ((NULL == fgets(line_buf, ORIGINAL_LINE_LEN - 1, fp)) ||
                '#' == line_buf[0])
        {
            continue;
        }

        line_buf[ORIGINAL_LINE_LEN - 1] = '\0';
        snprintf(tmp_buf, ORIGINAL_LINE_LEN, "%s", line_buf);
        // logprintf("every line: [%s]", line_buf);
        dict_id++;
        chinese_char_flag = 0;
        weight = 0.0;

        /** find query */
        p    = tmp_buf;
        find = tmp_buf;
        if (NULL != (p = strstr(tmp_buf, SEPARATOR)))
        {
            *p = '\0';
            find = p + 1;
        }
        //logprintf("every line: [%s]", line_buf);

        prefix_array.count = 0;
        k = 0;
        str_len = strlen(tmp_buf);
        for (i = 1; i <= MB_LENGTH
                && i < str_len && k < PREFIX_ARRAY_SIZE; i++)
        {
            /** Cleared buffer */
            memset(prefix, 0, sizeof(prefix));

            strtolower(tmp_buf, str_len, DEFAULT_ENCODING);
            result = cut_str(tmp_buf, prefix, sizeof(prefix), DEFAULT_ENCODING, i, "");
            if (! chinese_char_flag && result != i)
            {
                chinese_char_flag = 1;
            }

            k = prefix_array.count;
            snprintf(prefix_array.prefix[k], PREFIX_LEN, "%s", prefix);
            prefix_array.count++;
#if (_DEBUG)
            logprintf("prefix_array.prefix[%d] = %s, result: %d, str_len: %lu",
                (int)k, prefix_array.prefix[k], result, str_len);
#endif

            if (result >= str_len)
            {
                break;
            }
        }

        /** 全拼和简拼 */
        // TODO
        if (chinese_char_flag)
        {
            ;
        }

        /** find weight */
        p = find;
        if (NULL != (p = strstr(p, SEPARATOR)))
        {
            *p = '\0';
        }
        weight = find ? (float)atof(find) : 0.0;
#if (_DEBUG)
        logprintf("weight: %f, dict_id: %lu, prefix_array.count: %d",
            weight, dict_id, prefix_array.count);
#endif

        /** 建索引 */
        for (i = 0; i < prefix_array.count; i++)
        {
            hash_key = hash(prefix_array.prefix[i], gconfig.max_hash_table_size);
            if (hash_key < 0)
            {
                fprintf(stderr, "hash('%s') error.\n", prefix_array.prefix[i]);
                continue;
            }

            hash_exist = 0;
            hash_item = &(hash_table[hash_key]);
            if (hash_item && '\0' == hash_item->prefix[0])
            {
                hash_item->weight_array->weight_item[0].weight  = weight;
                hash_item->weight_array->weight_item[0].dict_id = dict_id;
                hash_item->weight_array->count = 1;
                snprintf(hash_item->prefix, QUERY_LEN, "%s", prefix_array.prefix[i]);

                continue;
            }
            while (hash_item)
            {
#if (_DEBUG)
                logprintf("hash_item->prefix:[%s] CMP prefix_array.data[%d] : [%s]",
                    hash_item->prefix, i, prefix_array.prefix[i]);
#endif
                if (hash_item->prefix[0]
                        && ((u_char)hash_item->prefix[0] == (u_char)prefix_array.prefix[i][0]))
                {
                    hash_exist = 1;
#if (_DEBUG)
                    logprintf("find exist prefix: [%s]", prefix_array.prefix[i]);
#endif
                    break;
                }
                hash_item = hash_item->next;
            }
            /** 如果 hash_key 存在,则将 dict_id 和 weight 写入, 并直接进入下一轮 */
            if (hash_exist)
            {
#if (_DEBUG)
                logprintf("multiple: [%s]", prefix_array.prefix[i]);
#endif
                k = hash_item->weight_array->count;
                if (k < DEFAULT_WEIGHT_ARRAY_SIZE)
                {
                    hash_item->weight_array->weight_item[k].weight  = weight;
                    hash_item->weight_array->weight_item[k].dict_id = dict_id;
                    hash_item->weight_array->count++;
                }
                continue;
            }
            /** 新元素,加入到 hash 表中,拉链 */
            if (NULL == hash_item)
            {
#if (_DEBUG)
                logprintf("---- at create new memory ---");
#endif
                hash_item = &(hash_table[hash_key]);
                while (hash_item->next)
                {
                    hash_item = hash_item->next;
                }

                size = sizeof(hash_list_ext_t);
                tmp_hash_item = (hash_list_ext_t *)malloc(size);
                if (NULL == tmp_hash_item)
                {
                    fprintf(stderr, "Can NOT malloc memory for tmp_hash_item, need size: %ld\n",
                        size);
                    ret = -1;
                    goto FATAL_ERROR;
                }

                size = sizeof(char) * QUERY_LEN;
                tmp_hash_item->prefix = (char *)malloc(size);
                if (NULL == tmp_hash_item->prefix)
                {
                    fprintf(stderr, "Can NOT malloc memory for tmp_hash_item->prefix, need size: %ld\n",
                        size);
                    ret = -1;
                    goto FATAL_ERROR;
                }

                size = sizeof(weight_array_t) * DEFAULT_WEIGHT_ARRAY_SIZE;
                tmp_hash_item->weight_array = (weight_array_t *)malloc(size);
                if (NULL == tmp_hash_item->weight_array)
                {
                    fprintf(stderr, "Can NOT malloc memory for tmp_hash_item->weight_array, need size: %ld\n",
                        size);
                    ret = -1;
                    goto FATAL_ERROR;
                }

                /** 新申请内存初始化 */
                tmp_hash_item->next = NULL;
                snprintf(tmp_hash_item->prefix, QUERY_LEN, "%s", prefix_array.prefix[i]);
                tmp_hash_item->weight_array->count = 1;
                tmp_hash_item->weight_array->weight_item[0].dict_id = dict_id;
                tmp_hash_item->weight_array->weight_item[0].weight  = weight;

                hash_item->next = tmp_hash_item;
                hash_item = tmp_hash_item;
            }
        }

        /** write dict data */
        snprintf(log_buf, LOG_BUF_LEN, "%lu\t%s", dict_id, line_buf);
        size = fwrite(log_buf, sizeof(char), strlen(log_buf), output_fp.dict_fp);
    }

    fprintf(stderr, "parse process completed.\n");

FATAL_ERROR:
    fclose(fp);

FINISH:
    return ret;
}

static int build(void)
{
    int ret = 0;
    int i = 0;
    int k = 0;
    hash_list_ext_t *hash_item = NULL;
    char log_buf[LOG_BUF_LEN];
    char *p = NULL;
    size_t size = 0;

    for (i = 0; i < gconfig.max_hash_table_size; i++)
    {
        hash_item = &(hash_table[i]);
        if (NULL == hash_item)
        {
            fprintf(stderr, "NO hash list for this key: %d\n", i);
            continue;
        }

        while (hash_item && hash_item->prefix[0])
        {
            p = log_buf;
            qsort(hash_item->weight_array->weight_item,
                  hash_item->weight_array->count, sizeof(weight_item_t), weight_cmp);

            p += snprintf(p, sizeof(log_buf) - (p - log_buf), "%s\t%i\t", hash_item->prefix, i);
            for (k = 0; k < hash_item->weight_array->count; k++)
            {
                p += snprintf(p, sizeof(log_buf) - (p - log_buf), "%lu,",
                        hash_item->weight_array->weight_item[k].dict_id);
            }
            p += snprintf(p, sizeof(log_buf) - (p - log_buf), "\n");
            size = fwrite(log_buf, sizeof(char), p - log_buf, output_fp.inverted_fp);

            hash_item = hash_item->next;
        }
    }

    return ret;
}

int main(int argc, char *argv[])
{
    int c;
    int t_opt;
    FILE *fp = NULL;

    size_t size = 0;

    signal_setup();
    init_config();

    while (-1 != (c = getopt(argc, argv,
        "s:"  /* max hash table size */
        "a:"  /* max dict table size */
        "v"   /* show version */
        "l:"  /* log level */
        "o:"  /* input original file name */
        "i:"  /* input inverted index file name */
        "x:"  /* input index dict file name */
        "h"  /* show usage */
    )))
    {
        switch (c)
        {
        case 's':
            size = (size_t)atoll(optarg);
            if (size > 0 && size > MAX_HASH_TABLE_SIZE)
            {
                gconfig.max_hash_table_size = size;
            }
            break;

        case 'a':
            size = (size_t)atoll(optarg);
            if (size > 0 && size > MAX_DICT_TABLE_SIZE)
            {
                gconfig.max_dict_table_size = size;
            }
            break;

        case 'l':
            t_opt = atoi(optarg);
            if (t_opt >= 0)
            {
                gconfig.log_level = t_opt;
            }
            break;

        case 'o':   /** original file */
            if (strlen(optarg))
            {
                snprintf(g_original_file, FILE_NAME_LEN,
                    "%s", optarg);
            }
            break;

        case 'i':   /** inverted index */
            if (strlen(optarg))
            {
                snprintf(gconfig.inverted_index, FILE_NAME_LEN,
                    "%s", optarg);
            }
            break;

        case 'x':   /** index dict */
            if (strlen(optarg))
            {
                snprintf(gconfig.index_dict, FILE_NAME_LEN,
                    "%s", optarg);
            }
            break;

        case 'v':
        case 'h':
            usage();
            exit(EXIT_SUCCESS);
        }
    }

#if (_DEBUG)
    logprintf("original file: [%s]", g_original_file);
    logprintf("inverted index file: [%s]", gconfig.inverted_index);
    logprintf("index dict file: [%s]", gconfig.index_dict);
    logprintf("hash_table_size: %lu", gconfig.max_hash_table_size);
#endif

    if (0 != init_hash_table())
    {
        fprintf(stderr, "init hash table fail, please check it out.\n");
        goto FINISH;
    }

    /*
    if (0 != init_brief_table())
    {
        fprintf(stderr, "init dict table fail, please check it out.\n");
        goto FINISH;
    }
    */

    /** 初始化输出文件指针 */
    fp = fopen(gconfig.inverted_index, "w");
    if (NULL == fp)
    {
        fprintf(stderr, "Can NOT open inverted file to write data: [%s]\n",
            gconfig.inverted_index);
        exit(-11);
    }
    output_fp.inverted_fp = fp;

    fp = fopen(gconfig.index_dict, "w");
    if (NULL == fp)
    {
        fprintf(stderr, "Can NOT open inverted file to write data: [%s]\n",
            gconfig.index_dict);
        exit(-11);
    }
    output_fp.dict_fp = fp;

    if (0 != parse_task())
    {
        fprintf(stderr, "parse_task fail.\n");
        goto FINISH;
    }

    if (0 != build())
    {
        fprintf(stderr, "build index and dict fail.\n");
        goto FINISH;
    }

FINISH:

    fclose(output_fp.inverted_fp);
    fclose(output_fp.dict_fp);

    return 0;
}

