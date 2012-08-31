/**
 * by: hy0kle@gmail.com
 * */
#include "util.h"
/**
 * global variables
 * */
config_t      gconfig;

hash_list_t  *index_hash_table = NULL;
index_dict_t *index_dict_table = NULL;
search_buf_t  search_buf;

static int search_process(const char *word, work_buf_t *work_buf)
{
    int ret  = 0;
    size_t i = 0;
    int k = 0;

    indext_t hash_key = 0;
    indext_t dict_id  = 0;
    size_t count      = 0;

    char lower_query[QUERY_LEN];
    char lower_dict_query[QUERY_LEN];

    hash_list_t *hash_item = NULL;

    if (NULL == word || NULL == work_buf)
    {
        ret = -1;
        goto FINISH;
    }

    snprintf(lower_query, QUERY_LEN, "%s", word);
    strtolower(lower_query, QUERY_LEN, "utf-8");
    hash_key = hash(lower_query, gconfig.max_hash_table_size);
    if (hash_key < 0)
    {
        work_buf->array_count = count;
        ret = -1;
        goto FINISH;
    }

    count = 0;
    hash_item = &(index_hash_table[hash_key]);
    while (hash_item && hash_item->index_item->size > 0)
    {
        i = 0;
        dict_id = hash_item->index_item->index_chain[i] - 1;
        snprintf(lower_dict_query, QUERY_LEN, "%s",
            index_dict_table[dict_id].dict_data->query_ext->queries[0]);
        strtolower(lower_dict_query, QUERY_LEN, "utf-8");
        //if ((unsigned char)lower_query[0] == (unsigned char)lower_dict_query[0])
        if (prefix_cmp(lower_dict_query, lower_query))
        {
            count = hash_item->index_item->size;
            goto FOUND;
        }

        for (k = 1; k < index_dict_table[dict_id].dict_data->query_ext->count; k++)
        {
            if ((unsigned char)lower_query[0] ==
                    (unsigned char)index_dict_table[dict_id].dict_data->query_ext->queries[k][0])
            {
                count = hash_item->index_item->size;
                goto FOUND;
            }
        }

        hash_item = hash_item->next;
    }
FOUND:
    if (! count)
    {
#if (_DEBUG)
        fprintf(stderr, "Can NOT find data in list.[%s]", word);
#endif
        work_buf->array_count = count;
        ret = -1;
        goto FINISH;
    }

    work_buf->array_count = count;
    for (i = 0; i < count; i++)
    {
        dict_id = hash_item->index_item->index_chain[i] - 1;
        snprintf(work_buf->dict_data[i].query, QUERY_LEN, "%s",
                index_dict_table[dict_id].dict_data->query_ext->queries[0]);
        snprintf(work_buf->dict_data[i].brief, BRIEF_LEN, "%s",
                index_dict_table[dict_id].dict_data->brief);
    }

FINISH:
    return ret;
}

static int build_html_body(char *tpl_buf, const work_buf_t *work_buf)
{
    int ret  = 0;
    size_t i = 0;
    char *p  = NULL;

    char query[QUERY_LEN];
    char brief[BRIEF_LEN];

    if (NULL == tpl_buf || NULL == work_buf)
    {
        ret = -1;
        goto FINISH;
    }

    p = tpl_buf;
    p += snprintf(p, TPL_BUF_LEN - (p - tpl_buf), "---simple suggestion system by \
prefix index---%s", CRLF);

    for (i = 0; i < work_buf->array_count; i++)
    {
        snprintf(query, QUERY_LEN, "%s", work_buf->dict_data[i].query);
        snprintf(brief, BRIEF_LEN, "%s", work_buf->dict_data[i].brief);
        str_replace(query, QUERY_LEN, "\\", "");
        str_replace(brief, BRIEF_LEN, "\\", "");

        p += snprintf(p, TPL_BUF_LEN - (p - tpl_buf), "query:%s, brief:%s%s",
            query, brief, CRLF);
    }

    if (0 == i)
    {
        p += snprintf(p, TPL_BUF_LEN - (p - tpl_buf), "Sorry, it is no match data, \
change keyword and try again.%s", CRLF);
    }

FINISH:
    return ret;
}

static int build_json_body(const char *callback, char *tpl_buf, const work_buf_t *work_buf)
{
    int ret = 0;
    int callback_validate = 0;

    size_t i = 0;
    char *p  = NULL;

    if (NULL == tpl_buf || NULL == work_buf)
    {
        ret = -1;
        goto FINISH;
    }

    if ('\0' != callback[0])
    {
        callback_validate = 1;
    }

    p = tpl_buf;
    p += snprintf(tpl_buf, TPL_BUF_LEN, "%s{\"stat\": \"200\", \"res_num\": \"%lu\", \"result\": [",
        callback_validate ? callback : "", work_buf->array_count);

    for (i = 0; i < work_buf->array_count; i++)
    {
        p += snprintf(p, TPL_BUF_LEN - (p - tpl_buf),
            "{\"query\": \"%s\", \"brief\": \"%s\"}%s",
            work_buf->dict_data[i].query,
            work_buf->dict_data[i].brief,
            i + 1 >= work_buf->array_count ? "" : ",");
    }
    p += snprintf(p, TPL_BUF_LEN - (p - tpl_buf), "]}%s",
        callback_validate ? ");" : "");

FINISH:
    return ret;
}

static int build_body(const int output_format, const char *callback, char *tpl_buf, const work_buf_t *work_buf)
{
    int ret = 0;

    switch (output_format)
    {
        case OUTPUT_AS_HTML:
            ret = build_html_body(tpl_buf, work_buf);
            break;

        case OUTPUT_AS_JSON:
            ret = build_json_body(callback, tpl_buf, work_buf);
            break;

        default:
            snprintf(tpl_buf, TPL_BUF_LEN, "NO output format, \
change set `format` and try again.%s", CRLF);
            break;
    }

    return ret;
}

/*
 * 处理模块
 * TODO: add lua to create JSON data.
 * TODO: add log logic.
 * TODO: add pid logic.
 *
*/
static void api_proxy_handler(struct evhttp_request *req, void *arg)
{
    // 初始化返回客户端的数据缓存
    struct evbuffer *buf;
    buf = evbuffer_new();

    work_buf_t *work_buf = NULL;
    char *tpl_buf = NULL;
    size_t token  = 0;

    /**
     * 0: default html
     * 1: json
     * */
    int error_flag = 0;
    int output_format  = OUTPUT_AS_HTML;
    char callback[64] = {0};
    char time_str[32] = {0};

    /** some action */
    //enum s_action_t s_action;
    //s_action = ACTION_NONE;

    /** apply work buf { */
    if (search_buf.current < gconfig.search_buf_size - 1)
    {
        token = search_buf.current++;
    }
    else
    {
        token = 0;
        search_buf.current = 0;
    }
    work_buf = &(search_buf.work_buf[token]);
    tpl_buf  = search_buf.tpl_buf[token];
    if (NULL == work_buf || NULL == tpl_buf)
    {
        error_flag = 1;
    }
#if (_DEBUG)
    logprintf("[error_flag]: %d, token: %lu", error_flag, token);
#endif
    /** end of apply memory for work buf } */

    /* 分析URL参数 */
    char *decode_uri = strdup((char*) evhttp_request_uri(req));
    struct evkeyvalq http_query;
    evhttp_parse_query(decode_uri, &http_query);

#if (_DEBUG)
    logprintf("uri: %s", decode_uri);
#endif

    /** free memory */
    free(decode_uri);

#if (_DEBUG)
#ifdef TAILQ_FOREACH
    //遍历整个uri的对应关系值
    {
        logprintf("--- foreach uri ---");
        struct evkeyval *header;
        TAILQ_FOREACH(header, &http_query, next) {
            logprintf("%s: %s", header->key, header->value);
        }
        logprintf("--- end uri ---");
    }
    //遍历整个请求头.
    {
        logprintf("---- foreach request header ----");
        struct evkeyvalq *input_headers = evhttp_request_get_input_headers(req);
        struct evkeyval *header;
        TAILQ_FOREACH(header, input_headers, next) {
            logprintf("%s: %s", header->key, header->value);
        }
        logprintf("---- end request header ----");
    }
#endif
#endif

    get_localtime_str(time_str, sizeof(time_str));

    /* 接收 GET 解析参数 { */
    //const char *http_input_name = evhttp_find_header(&http_query, "name");
    const char *uri_format    = evhttp_find_header(&http_query, "format");
    const char *uri_callback  = evhttp_find_header(&http_query, "callback");
    const char *word   = evhttp_find_header(&http_query, "word");
    //const char *action = evhttp_find_header(&http_query, "action");

    if (uri_format && 0 == strncmp(uri_format, FORMAT_JSON, sizeof(FORMAT_JSON) - 1))
    {
        output_format = OUTPUT_AS_JSON;
    }
    if (uri_callback && strlen(uri_callback))
    {
        snprintf(callback, sizeof(callback), "window.%s && window.%s(", uri_callback, uri_callback);
    }

    // switch s_action
    search_process(word, work_buf);
    /** end get and parse get parameter }*/
    build_body(output_format, callback, tpl_buf, work_buf);

    //处理输出header头
    evhttp_add_header(req->output_headers, "Content-Type", output_format ?
        "application/json; charset=UTF-8" :
            "text/html; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Status", "200 OK");
    evhttp_add_header(req->output_headers, "Connection", "keep-alive");
    evhttp_add_header(req->output_headers, "Cache-Control", "no-cache");
    evhttp_add_header(req->output_headers, "Expires", "-1");
    evhttp_add_header(req->output_headers, "Server", "lhs");    /** libevent http server */

    //处理输出数据
    evbuffer_add_printf(buf, "%s", tpl_buf);

    //返回code 200
    evhttp_send_reply(req, HTTP_OK, "OK", buf);

    //释放内存
    evhttp_clear_headers(&http_query);
    evbuffer_free(buf);

    return;
}

static void init_config(void)
{
    gconfig.do_daemonize = 1;
    gconfig.port         = DEFAULT_PORT;
    gconfig.timeout      = HTTP_TIMEOUT;
    gconfig.log_level    = 4;
    gconfig.max_hash_table_size = MAX_HASH_TABLE_SIZE;
    gconfig.max_dict_table_size = MAX_DICT_TABLE_SIZE;
    gconfig.search_buf_size     = SEARCH_BUF_SIZE;
    snprintf(gconfig.hostname, HOST_NAME_LEN, "%s", DEFAULT_LISTEN);
    snprintf(gconfig.inverted_index, FILE_NAME_LEN, "%s", DEFAULT_INVERTED_INDEX);
    snprintf(gconfig.index_dict, FILE_NAME_LEN, "%s", DEFAULT_INDEX_DICT);

    return;
}

static void usage(void)
{
    printf(PACKAGE " " VERSION "\n");
    printf("-s <num>      max hash table size(default: %d)\n", MAX_HASH_TABLE_SIZE);
    printf("-a <num>      max dict table size(default: %d)\n", MAX_DICT_TABLE_SIZE);
    printf("-f <num>      search buffer array count(default: %d)\n", SEARCH_BUF_SIZE);
    printf("-p <num>      TCP port number to listen on (default: %d)\n", DEFAULT_PORT);
    printf("-d [0|1]      cli or run as a daemon\n"
           "-H <hostname> interface to listen on (default: INADDR_ANY, all addresses)\n"
           "-t <timeout>  set HTTP timeout\n"
           "-v            show version and help\n"
           "-l <level>    set log lever\n"
           "-i <file>     set inverted index file name and path, ABS path is better\n"
           "-x <file>     set index dict file name and path, ABS path is better\n"
           "-j <file>     set lua tpl to create json\n"
           "-m <file>     set lua tpl to create html\n"
           "-h            show this help and exit\n");

    return;
}

static int load_index(void)
{
    int   ret= 0;
    int   k  = 0;
    FILE *fp = NULL;
    char *file_name = NULL;
    char  line[READ_LINE_BUF_LEN] = {0};
    char *find = NULL;
    char *p    = NULL;

    indext_t dict_id   = 0;
    size_t i = 0;
    size_t array_count = 0;
    size_t array_index = 0;
    size_t size = 0;

    int queries_num = 0;
    char queries[QUERY_EXT_SIZE][QUERY_LEN];
    char brief[BRIEF_LEN] = {0};

    hash_list_t *hash_item     = NULL;
    hash_list_t *tmp_hash_item = NULL;

    /** load inverted index data { */
    /**
     * File field format
     * prefix\tindex(hash key)\tdict-id-m,...dict-id-n,\n
     * */
    file_name = gconfig.inverted_index;
#if (_DEBUG)
    logprintf("inverted_index: [%s]", file_name);
#endif
    fp = fopen(file_name, "r");
    if (! fp)
    {
        fprintf(stderr, "Can NOT open inverted index file: %s\n",
            file_name);
        ret = -1;
        goto FINISH;
    }

    while(! feof(fp))
    {
        if (NULL == fgets(line, READ_LINE_BUF_LEN - 1, fp))
        {
            continue;
        }

        line[READ_LINE_BUF_LEN - 1] = '\0';
        //logprintf("every line: %s", line);
        p = line;

        /** skip query */
        find = strstr(p, SEPARATOR);
        if (find)
        {
            *find = '\0';
        }
        p = find + 1;

        find = strstr(p, SEPARATOR);
        if (find)
        {
            *find = '\0';
        }
        array_index = (size_t)atoll(p) % gconfig.max_hash_table_size;
        //logprintf("inverted index: %lu", array_index);
        hash_item = &(index_hash_table[array_index]);
        while (hash_item->index_item->size > 0)
        {
            size = sizeof(hash_list_t);
            tmp_hash_item = (hash_list_t *)malloc(size);
            if (NULL == tmp_hash_item)
            {
                fprintf(stderr, "Can NOT malloc memory for tmp_hash_item, need size: %ld\n",
                    size);
                ret = -1;
                goto FINISH;
            }
            tmp_hash_item->next = NULL;

            size = sizeof(index_item_t);
            tmp_hash_item->index_item = (index_item_t *)malloc(size);
            if (NULL == tmp_hash_item->index_item)
            {
                fprintf(stderr, "Can NOT malloc memory for tmp_hash_item->index_item, need size: %ld\n",
                    size);
                ret = -1;
                goto FINISH;
            }
            tmp_hash_item->index_item->size = 0;

            size = sizeof(indext_t) * SINGLE_INDEX_SIZE;
            tmp_hash_item->index_item->index_chain = (indext_t *)malloc(size);
            if (NULL == tmp_hash_item->index_item->index_chain)
            {
                fprintf(stderr, "Can NOT malloc memory for tmp_hash_item->index_item->index_chain, need size: %ld\n",
                    size);
                ret = -1;
                goto FINISH;
            }

            hash_item->next = tmp_hash_item;
            hash_item = tmp_hash_item;
        }

        p = find + 1;
        for (i = 0, array_count = 0; i < SINGLE_INDEX_SIZE; i++)
        {
            find = strstr(p, ",");
            if (find)
            {
                *find = '\0';
                dict_id = (indext_t)atoll(p);
                if (! (dict_id > 0))
                {
                    continue;
                }
                //logprintf("every dict id: %lu", dict_id);
                hash_item->index_item->index_chain[array_count] = dict_id;

                p = find + 1;
                array_count++;
            }
        }
        hash_item->index_item->size = array_count;
#if (_DEBUG)
        logprintf("index_hash_table[%lu].size = %lu", array_index, array_count);
#endif
    }

    fclose(fp);
    /** end for inverted index } */

    /** load dict data { */
    /**
     * index_dict file format
     * dict-id\tquery-word\thot\tbrief-info\n
     * */
    file_name = gconfig.index_dict;
#if (_DEBUG)
    logprintf("index_dict: [%s]", file_name);
#endif
    fp = fopen(file_name, "r");
    if (! fp)
    {
        fprintf(stderr, "Can NOT open dict data file: [%s]\n", file_name);
        ret = -1;
        goto FINISH;
    }

    while (! feof(fp))
    {
        if (NULL == fgets(line, READ_LINE_BUF_LEN - 1, fp))
        {
            continue;
        }

        line[READ_LINE_BUF_LEN - 1] = '\0';
        //logprintf("every line: %s", line);
        p = line;
        find = line;

        find = strstr(p, SEPARATOR);
        if (NULL == find)
        {
            continue;
        }
        *find = '\0';
        array_index = (size_t)atoll(p);
#if (_DEBUG)
        logprintf("array_index: %lu", array_index);
#endif
        /** @: Important fault-tolerant */
        if (! (array_index > 0))
        {
            continue;
        }
        else if (array_index > gconfig.max_dict_table_size)
        {
#if (_DEBUG)
            logprintf("The dict number out of max dict tabel size: %lu", gconfig.max_dict_table_size);
#endif
            break;
        }
        /** hash key 和字典序差 1,为了使数组下标从 0 开始而考虑 */
        array_index -= 1;

        /** 处理 query, 包含有全拼和简拼 { */
        queries_num = 0;
        for (k = 0; k < QUERY_EXT_SIZE; k++)
        {
            p = find + 1;
            find = strstr(p, SEPARATOR);
            if (NULL == find)
            {
                ret = -1;
                goto LOAD_ERROR;
            }
            *find = '\0';

            if (strlen(p))
            {
                queries_num++;
                snprintf(queries[k], QUERY_LEN, "%s", p);
            }
        }
        if (0 == queries_num)
        {
            ret = -1;
            goto LOAD_ERROR;
        }
        size = sizeof(query_ext_t);
        index_dict_table[array_index].dict_data->query_ext= (query_ext_t *)malloc(size);
        if (NULL == index_dict_table[array_index].dict_data->query_ext)
        {
            fprintf(stderr, "Can NOT malloc memory for index_dict_table[%lu].dict_data->query_ext, need size: %ld\n",
                array_index, size);
            ret = -1;
            goto LOAD_ERROR;
        }
        index_dict_table[array_index].dict_data->query_ext->count = queries_num;
        size = sizeof(char *) * queries_num;
        index_dict_table[array_index].dict_data->query_ext->queries = (char **)malloc(size);
        if (NULL == index_dict_table[array_index].dict_data->query_ext->queries)
        {
            fprintf(stderr, "Can NOT malloc memory for \
index_dict_table[%lu].dict_data->query_ext->queries, need size: %ld\n",
                array_index, size);
            ret = -1;
            goto LOAD_ERROR;
        }
        for (k = 0; k < queries_num; k++)
        {
            size = strlen(queries[k]) + 1;
            index_dict_table[array_index].dict_data->query_ext->queries[k] = (char *)malloc(size);
            if (NULL == index_dict_table[array_index].dict_data->query_ext->queries[k])
            {
                fprintf(stderr, "Can NOT malloc memory for index_\
dict_table[%lu].dict_data->query_ext->queries[%d], need size: %ld\n",
                    array_index, k, size);
                ret = -1;
                goto LOAD_ERROR;
            }
            snprintf(index_dict_table[array_index].dict_data->query_ext->queries[k], size, "%s", queries[k]);
        }
        /** end } */

        /** skip hot field */
        p = find + 1;
        find = strstr(p, SEPARATOR);
        if (find)
        {
            *find = '\0';
        }
        p = find + 1;

        /** trim last \n */
        find = strstr(p, "\n");
        if (find)
        {
            *find = '\0';
        }
        snprintf(brief, BRIEF_LEN, "%s", p);
        //logprintf("every brief: [%s]", p);

        size = strlen(brief) + 1;
        index_dict_table[array_index].dict_data->brief = (char *)malloc(size);
        if (NULL == index_dict_table[array_index].dict_data->brief)
        {
            fprintf(stderr, "Can NOT malloc memory for index_dict_table[%lu].dict_data->brief, need size: %ld\n",
                array_index, size);
            ret = -1;
            goto LOAD_ERROR;
        }
        snprintf(index_dict_table[array_index].dict_data->brief, size, "%s", brief);
#if (_DEBUG)
        logprintf("index_dict_table[%lu].query = %s", array_index, queries[0]);
        logprintf("index_dict_table[%lu].brief = %s", array_index, brief);
#endif
    }

LOAD_ERROR:
    fclose(fp);
    /** end of for dict }*/
FINISH:
    return ret;
}

/**
 * 申请程序需要的内存空间
 * */
static int init_search_library(void)
{
    int ret = 0;
    int i = 0;
    int k = 0;
    size_t size = 0;
    size_t sub_size = 0;

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

    /** 申请正排表的内存空间 */
    size = sizeof(index_dict_t) * gconfig.max_dict_table_size;
    index_dict_table = (index_dict_t *)malloc(size);
    if (NULL == index_dict_table)
    {
        fprintf(stderr, "Can NOT malloc memory for index dict, need size: %ld\n",
            size);
        ret = -1;
        goto FINISH;
    }
    for (i = 0; i < gconfig.max_dict_table_size; i++)
    {
        /**
         * 为了有效使用内存, query 和 brief 采用不定长方式,
         * 所以在真正需要拷贝数据时才申请刚好容纳下 brief 的空间
         * 每个 query 和 brief 占用内存是固定的 */
        /**
        size = sizeof(char) * QUERY_LEN;
        index_dict_table[i].query = (char *)malloc(size);
        if (NULL == index_dict_table[i].query)
        {
            fprintf(stderr, "Can NOT malloc memory for index_dict_table[%d].query, need size: %ld\n",
                i, size);
            ret = -1;
            goto FINISH;
        }
        */
        // index_dict_table[i].query_ext = NULL;

        /**
        size = sizeof(char) * BRIEF_LEN;
        index_dict_table[i].brief = (char *)malloc(size);
        if (NULL == index_dict_table[i].brief)
        {
            fprintf(stderr, "Can NOT malloc memory for index_dict_table[%d].brief, need size: %ld\n",
                i, size);
            ret = -1;
            goto FINISH;
        }
        */
        // index_dict_table[i].brief = NULL;

        size = sizeof(dict_data_t);
        index_dict_table[i].dict_data = (dict_data_t *)malloc(size);
        if (NULL == index_dict_table[i].dict_data)
        {
            fprintf(stderr, "Can NOT malloc memory for index_dict_table[%d].dict_data, need size: %ld\n",
                i, size);
            ret = -1;
            goto FINISH;
        }
        index_dict_table[i].dict_data->brief     = NULL;
        index_dict_table[i].dict_data->query_ext = NULL;
    }

    /**
     * 申请工作进程的内存池,避免动态内存分配
     * 但如果并发量上来,不加锁有可能触发数据错乱
     * 加锁则影响性能,开到足够大能降低概率,不能免除
     * 我的理解 ^_*
     * */
#if (_DEBUG)
    logprintf("apply memory for search_buf.dict_data.");
#endif
    search_buf.current   = 0;
    size = sizeof(work_buf_t) * gconfig.search_buf_size;
    search_buf.work_buf = (work_buf_t *)malloc(size);
    if (NULL == search_buf.work_buf)
    {
        fprintf(stderr, "Can NOT malloc memory for search_buf->work_buf[%lu], need size: %ld\n",
            gconfig.search_buf_size, size);
        ret = -1;
        goto FINISH;
    }
    size = sizeof(dict_ext_t) * SINGLE_INDEX_SIZE;
    for (i = 0; i < gconfig.search_buf_size; i++)
    {
        search_buf.work_buf[i].dict_data = (dict_ext_t *)malloc(size);
        if (NULL == search_buf.work_buf[i].dict_data)
        {
            fprintf(stderr, "Can NOT malloc memory for search_buf.work_buf_t[%d].dict_data, need size: %ld\n",
                i, size);
            ret = -1;
            goto FINISH;
        }
        for (k = 0; k < SINGLE_INDEX_SIZE; k++)
        {
            /** cache buf, query */
            sub_size = sizeof(char) * QUERY_LEN;
            search_buf.work_buf[i].dict_data[k].query = (char *)malloc(sub_size);
            if (NULL == search_buf.work_buf[i].dict_data[k].query)
            {
                fprintf(stderr, "Can NOT malloc memory for \
search_buf.work_buf[%d].dict_data[%d].query, need size: %lu\n",
                    i, k, sub_size);
                ret = -1;
                goto FINISH;
            }
            /** cache buf, brief */
            sub_size = sizeof(char) * BRIEF_LEN;
            search_buf.work_buf[i].dict_data[k].brief = (char *)malloc(sub_size);
            if (NULL == search_buf.work_buf[i].dict_data[k].brief)
            {
                fprintf(stderr, "Can NOT malloc memory for \
search_buf.work_buf[%d].dict_data[%d].brief, need size: %lu\n",
                    i, k, sub_size);
                ret = -1;
                goto FINISH;
            }
        }
    }

#if (_DEBUG)
    logprintf("apply memory for search_buf.tpl_buf.");
#endif
    size = sizeof(char *) * gconfig.search_buf_size;
    search_buf.tpl_buf = (char **)malloc(size);
    if (NULL == search_buf.tpl_buf)
    {
        fprintf(stderr, "Can NOT malloc memory for search_buf->tpl_buf, need size: %ld\n",
            size);
        ret = -1;
        goto FINISH;
    }
    size = sizeof(char) * TPL_BUF_LEN;
    for (i = 0; i < gconfig.search_buf_size; i++)
    {
        search_buf.tpl_buf[i] = (char *)malloc(size);
        if (NULL == search_buf.tpl_buf[i])
        {
            fprintf(stderr, "Can NOT malloc memory for search_buf->tpl_buf[%d], need size: %ld\n",
                i, size);
            ret = -1;
            goto FINISH;
        }
    }

FINISH:
    return ret;
}

static void print_gconfig(void)
{
    fprintf(stderr, "---gconfig---\n");
    fprintf(stderr, "gconfig.do_daemonize = %d\n", gconfig.do_daemonize);
    fprintf(stderr, "gconfig.port         = %d\n", gconfig.port);
    fprintf(stderr, "gconfig.timeout      = %d\n", gconfig.timeout);
    fprintf(stderr, "gconfig.log_level    = %d\n", gconfig.log_level);
    fprintf(stderr, "gconfig.max_hash_table_size = %lu\n", gconfig.max_hash_table_size);
    fprintf(stderr, "gconfig.max_dict_table_size = %lu\n", gconfig.max_dict_table_size);
    fprintf(stderr, "gconfig.search_buf_size     = %lu\n", gconfig.search_buf_size);
    fprintf(stderr, "gconfig.hostname:       [%s]\n", gconfig.hostname);
    fprintf(stderr, "gconfig.inverted_index: [%s]\n", gconfig.inverted_index);
    fprintf(stderr, "gconfig.index_dict:     [%s]\n", gconfig.index_dict);
    fprintf(stderr, "---end gconfig---\n");

    return;
}

int main(int argc, char** argv)
{
    int c;
    int t_opt;
    size_t size;
    struct evhttp *httpd = NULL;
    /**
    char *proxy_listen   = DEFAULT_LISTEN;    //绑定所有ip
    int proxy_port       = DEFAULT_PORT;      //端口号
    int proxy_settings_timeout = HTTP_TIMEOUT;     //http请求超时时间
    */

    signal_setup();

    init_config();
    /* process arguments */
    /*"c:"   config file name  */
    while (-1 != (c = getopt(argc, argv,
        "d:"  /* work as daemon process */
        "s:"  /* max hash table size */
        "a:"  /* max dict table size */
        "f:"  /* search buffer array count */
        "H:"  /* http hostname -i */
        "p:"  /* http listen port  */
        "t:"  /* http timeout */
        "v"   /* show version */
        "l:"  /* log level */
        "i:"  /* input inverted index file name */
        "x:"  /* input index dict file name */
        "j:"  /* create JSON by lua tpl */
        "m:"  /* create html by lua tpl */
        "h"  /* show usage */
    )))
    {
        switch (c)
        {
        case 'd':
            if (strlen(optarg))
            {
                gconfig.do_daemonize = atoi(optarg);
            }
            break;

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

        case 'f':
            size = (size_t)atoll(optarg);
            if (size > 0 && size > SEARCH_BUF_SIZE)
            {
                gconfig.search_buf_size = size;
            }
            break;

        case 'H':
            if (strlen(optarg))
            {
                snprintf(gconfig.hostname, HOST_NAME_LEN, "%s", optarg);
            }
            else
            {
                fprintf(stderr, "-H need hostname, please set it.\n");
                exit(EXIT_SUCCESS);
            }
            break;

        case 'p':
            t_opt = atoi(optarg);
            if (t_opt > 0)
            {
                gconfig.port = t_opt;
            }
            break;

        case 't':
            t_opt = atoi(optarg);
            if (t_opt >= 0 && t_opt <= 60 )
            {
                gconfig.timeout = t_opt;
            }
            break;

        case 'l':
            t_opt = atoi(optarg);
            if (t_opt >= 0)
            {
                gconfig.log_level = t_opt;
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

        case 'j':   /** lua json tpl */
            break;

        case 'm':   /** lua html tpl */
            break;

            case 'v':

        case 'h':
            usage();
            exit(EXIT_SUCCESS);
        }
    }

#if (DAEMON)
    if (gconfig.do_daemonize && (-1 == daemonize(0, 1)))
    {
        fprintf(stderr, "failed to daemon() in order to daemonize\n");
        exit(EXIT_FAILURE);
    }
#endif

    print_gconfig();

    if (0 != init_search_library())
    {
        fprintf(stderr, "init search library has something wrong.\n");
        exit(-1);
    }

    if (0 != load_index())
    {
        fprintf(stderr, "load index and dict data has wrong.\n");
        exit(-1);
    }

    fprintf(stderr, "Init is OK, it will work for you. ^_*\n");

    /** 初始化事件 */
    event_init();
    /** 初始化监听ip和端口 */
    // httpd = evhttp_start(proxy_listen, proxy_port);
    httpd = evhttp_start(gconfig.hostname, gconfig.port);
    if (NULL == httpd)
    {
        fprintf(stderr, "[Error]: Unable to listen on %s:%d\n",
            gconfig.hostname, gconfig.port);
        exit(1);
    }

    // 设置http连接超时时间
    //evhttp_set_timeout(httpd, proxy_settings_timeout);
    evhttp_set_timeout(httpd, gconfig.timeout);
    // 设置请求到达后的回调函数
    evhttp_set_gencb(httpd, api_proxy_handler, NULL);
    // libevent 循环处理事件
    event_dispatch();
    // 释放资源
    evhttp_free(httpd);

    return 0;
}

