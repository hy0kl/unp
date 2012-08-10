#include <pthread.h>
#include "build.h"
#include "pinyin_data.h"
/**
 * global variables
 * */
config_t      gconfig;

hash_list_t  *index_hash_table  = NULL;

indext_t  g_dict_id = 0;
char      g_original_file[FILE_NAME_LEN] = DEFAULT_ORIGINAL_FILE;
int       g_parse_completed = 0;
int pipe_fp[2];
/** END for global vars */

static void init_config()
{
    gconfig.log_level    = 4;
    gconfig.max_hash_table_size = MAX_HASH_TABLE_SIZE;
    snprintf(gconfig.hostname, HOST_NAME_LEN, "%s", DEFAULT_LISTEN);
    snprintf(gconfig.inverted_index, FILE_NAME_LEN, "%s", DEFAULT_INVERTED_INDEX);
    snprintf(gconfig.index_dict, FILE_NAME_LEN, "%s", DEFAULT_INDEX_DICT);

    return;
}

static int init_hash_table()
{
    int ret = 0;
    int i   = 0;
    size_t size = 0;

    /** 申请倒排表的内存空间 */
    size = sizeof(hash_list_t) * gconfig.max_hash_table_size;
    index_hash_table = (hash_list_t *)malloc(size);
    if (NULL == index_hash_table)
    {
        fprintf(stderr, "Can NOT malloc memory for hash table, need size: %ld\n",
            size);
        ret = -1;
        goto FINISH;
    }
    //memset(index_hash_table, 0, gconfig.max_hash_table_size);
    for (i = 0; i < gconfig.max_hash_table_size; i++)
    {
        index_hash_table[i].next = NULL;

        size = sizeof(index_item_t);
        index_hash_table[i].index_item = (index_item_t *)malloc(size);
        if (NULL == index_hash_table[i].index_item)
        {
            fprintf(stderr, "Can NOT malloc memory for ndex_hash_table[%d].index_item, need size: %ld\n",
                i, size);
            ret = -1;
            goto FINISH;
        }

        size = sizeof(indext_t) * SINGLE_INDEX_SIZE;
        index_hash_table[i].index_item->index_chain = (indext_t *)malloc(size);
        if (NULL == index_hash_table[i].index_item->index_chain)
        {
            fprintf(stderr, "Can NOT malloc memory for ndex_hash_table[%d].index_item->index_chain, need size: %ld\n",
                i, size);
            ret = -1;
            goto FINISH;
        }
        index_hash_table[i].index_item->size = 0;
    }

FINISH:
    return ret;
}

static void usage()
{
    printf(BUILD_PACKAGE " " BUILD_VERSION "\n");
    printf("-s <num>      max hash table size(default: %d)\n", MAX_HASH_TABLE_SIZE);
    printf("-v            show version and help\n"
           "-l <level>    set log lever\n"
           "-o <file>     set original file name adn path, ABS path is better\n"
           "-i <file>     set inverted index file name and path, ABS path is better\n"
           "-x <file>     set index dict file name and path, ABS path is better\n"
           "-h            show this help and exit\n");

    return;
}

static void parse_task()
{
    char line_buf[ORIGINAL_LINE_LEN];
    char prefix[PREFIX_LEN];
    //char tmp_buf[QUERY_LEN];
    char *p = NULL;

    FILE *fp = NULL;
    int   i  = 0;
    int   k  = 0;
    int result = 0;
    int chinese_char_flag = 0;
    //size_t size = 0;

    close(pipe_fp[PIPE_READER]);

    indext_t dict_id   = 0;
    task_queue_t task;

    fp = fopen(g_original_file, "r");
    if (! fp)
    {
        fprintf(stderr, "Can open original file: [%s]\n", g_original_file);

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
        // logprintf("every line: [%s]", line_buf);

        dict_id++;
        chinese_char_flag = 0;

        /** 创建工作队列单个任务 */
        memset(&task, 0, sizeof(task_queue_t));

        task.dict_id = dict_id;
        snprintf(task.original_line, ORIGINAL_LINE_LEN, "%s", line_buf);

        if (NULL != (p = strstr(line_buf, SEPARATOR)))
        {
            *p = '\0';
        }
        //logprintf("every line: [%s]", line_buf);
        k = task.prefix_array.count;
        for (i = 1; i <= MB_LENGTH && k < PREFIX_ARRAY_SIZE; i++)
        {
            /** Cleared buffer */
            memset(prefix, 0, sizeof(prefix));

            strtolower(line_buf, strlen(line_buf), DEFAULT_ENCODING);
            result = cut_str(line_buf, prefix, sizeof(prefix), DEFAULT_ENCODING, i, "");
            if (! chinese_char_flag && result != i)
            {
                chinese_char_flag = 1;
            }

            k = task.prefix_array.count;
            snprintf(task.prefix_array.data[k], PREFIX_LEN, "%s", prefix);
            task.prefix_array.count++;
            logprintf("every prefix: [%s]", prefix);
        }

        /** 全拼和简拼 */
        // TODO
        if (chinese_char_flag)
        {
            ;
        }

        write(pipe_fp[PIPE_WRITER], &task, sizeof(task_queue_t));
    }

    task.dict_id = 0;
    write(pipe_fp[PIPE_WRITER], &task, sizeof(task_queue_t));

    fprintf(stderr, "parse process completed.\n");
    fclose(fp);

FINISH:
    return;
}

static void handle_task()
{
    task_queue_t task;

    close(pipe_fp[PIPE_WRITER]);
    while (1)
    {
        memset(&task, 0, sizeof(task_queue_t));
        read(pipe_fp[PIPE_READER], &task, sizeof(task_queue_t));

        if (0 == task.dict_id)
        {
            fprintf(stderr, "handle process completed.\n");
            break;
        }
        logprintf("recv data: [%s]", task.original_line);

        usleep((useconds_t)(5000));
    }

    return;
}

int main(int argc, char *argv[])
{
    int c;
    int t_opt;

    size_t size = 0;
    pid_t  pid;

    signal_setup();
    init_config();

    while (-1 != (c = getopt(argc, argv,
        "s:"  /* max hash table size */
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
        fprintf(stderr, "init_hash_table fail.\n");
        exit(-1);
    }
#if (_DEBUG)
    else
    {
        logprintf("init hash table success.");
    }
#endif

    /** init pipe */
    if (0 != pipe(pipe_fp))
    {
        perror("pipe()");
        exit(1);
    }

    if (-1 == (pid = fork()))
    {
        fprintf(stderr, "fork() fail, please check it out.\n");
        exit(-2);
    }
    else if (0 == pid)
    {
#if (_DEBUG)
        logprintf("I am in child process to parse task.");
#endif
        parse_task();
    }
    else
    {
#if (_DEBUG)
        logprintf("I am in parent process to handle every task.");
#endif
        handle_task();
    }

    return 0;
}

