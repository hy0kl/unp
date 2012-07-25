#include "util.h"
/**
 * global variables
 * */
config_t      gconfig;

index_term_t *index_hash_table = NULL;
index_dict_t *index_dict_table = NULL;
search_buf_t  search_buf;

/*
 * 处理模块
 * gw event-http.c -o http-sever -levent
 * http://localhost:DEFAULT_PORT/?name=test
 * TODO: add getopt() and gconf.
 * TODO: add lua to create JSON data.
 * TODO: add log logic.
 * TODO: add search logic.
*/
static void api_proxy_handler(struct evhttp_request *req, void *arg)
{
    //初始化返回客户端的数据缓存
    struct evbuffer *buf;
    buf = evbuffer_new();

    /**
     * 0: default html
     * 1: json
     * */
    int output_format  = 0;
    int callback_validate = 0;
    char callback[16] = {0};
    char time_str[32] = {0};

    /* 分析URL参数 */
    char *decode_uri = strdup((char*) evhttp_request_uri(req));
    struct evkeyvalq http_query;
    evhttp_parse_query(decode_uri, &http_query);

    logprintf("uri: %s", decode_uri);
    free(decode_uri);

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

    get_localtime_str(time_str, sizeof(time_str));

    //接收GET表单参数name
    const char *http_input_name = evhttp_find_header(&http_query, "name");
    const char *uri_format      = evhttp_find_header(&http_query, "format");
    const char *uri_callback    = evhttp_find_header(&http_query, "callback");
    const char *word            = evhttp_find_header(&http_query, "word");

    if (uri_format && 0 == strncmp(uri_format, FORMAT_JSON, sizeof(FORMAT_JSON) - 1))
    {
        output_format = 1;
    }
    if (uri_callback && strlen(uri_callback))
    {
        snprintf(callback, sizeof(callback), "%s && %s(", uri_callback, uri_callback);
        callback_validate = 1;
    }

    //处理输出header头
    if (output_format)
    {
        evhttp_add_header(req->output_headers, "Content-Type", "application/x-javascript; charset=UTF-8");
    }
    else
    {
        evhttp_add_header(req->output_headers, "Content-Type", "text/html; charset=UTF-8");
    }
    evhttp_add_header(req->output_headers, "Status", "200 OK");
    evhttp_add_header(req->output_headers, "Connection", "keep-alive");
    evhttp_add_header(req->output_headers, "Cache-Control", "no-cache");
    evhttp_add_header(req->output_headers, "Expires", "-1");
    evhttp_add_header(req->output_headers, "Server", "lhs");    /** libevent http server */

    //处理输出数据
    if (output_format)
    {
        evbuffer_add_printf(buf, "%s{\"stat\": 200,\
\"info\": {\"notice\": \"welcome to libevent word.\",\
\"version\": \"%s\",\
\"time\": \"%s\"\
},\
\"language\": [\"c\", \"php\", \"javascript\", \"shell\",\
\"python\", \"lua\", \"css/html\", \"sql\"]\
}%s", callback_validate ? callback : "", VERSION,
        time_str, callback_validate ? ")" : "");
    }
    else
    {
        evbuffer_add_printf(buf, "<html><body><head>\
<title>Libevent Http Sever</title>\
</head><body>");
        evbuffer_add_printf(buf, "PROXY VERSION %s%s\n", VERSION, CRLF);
        evbuffer_add_printf(buf, "------------------------------%s\n", CRLF);
        evbuffer_add_printf(buf, "YOU PASS name: %s%s\n", http_input_name ? http_input_name : "NONE", CRLF);
        evbuffer_add_printf(buf, "Time: %s%s\n", time_str, CRLF);
        if (NULL != word)
        {
            evbuffer_add_printf(buf, "word: %s, hash('%s') = %lu, table size = %d %s\n",
                word, word, hash_word(word, gconfig.max_hash_table_size),
                (int)gconfig.max_hash_table_size, CRLF);
        }
        evbuffer_add_printf(buf, "</body></html>");
    }

    //返回code 200
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    //释放内存
    evhttp_clear_headers(&http_query);
    evbuffer_free(buf);
}

static void init_config()
{
    gconfig.do_daemonize = 1;
    gconfig.port         = DEFAULT_PORT;
    gconfig.timeout      = HTTP_TIMEOUT;
    gconfig.log_level    = 4;
    gconfig.max_hash_table_size = MAX_HASH_TABLE_SIZE;
    snprintf(gconfig.hostname, HOST_NAME_LEN, "%s", DEFAULT_LISTEN);
    snprintf(gconfig.inverted_index, FILE_NAME_LEN, "%s", DEFAULT_INVERTED_INDEX);
    snprintf(gconfig.index_dict, FILE_NAME_LEN, "%s", DEFAULT_INDEX_DICT);

    return;
}

static void usage()
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
           "-i <file>     set inverted index file name and path\n"
           "-x <file>     set index dict file name and path\n"
           "-j <file>     set lua tpl to create json\n"
           "-m <file>     set lua tpl to create html\n"
           "-h            show this help and exit\n");

    return;
}

static int load_index()
{
    int ret = 0;

    return ret;
}

/**
 * 申请程序需要的内存空间
 * */
static int init_search_library()
{
    int ret = 0;
    int i = 0;

    /** 申请倒排表的内存空间 */
    index_hash_table = (index_term_t *)malloc(sizeof(index_term_t) * gconfig.max_hash_table_size);
    if (NULL == index_hash_table)
    {
        fprintf(stderr, "Can NOT malloc memory for hash table, need size: %ld\n",
            sizeof(index_term_t) * gconfig.max_hash_table_size);
        ret = -1;
        goto FINISH;
    }

    /** 申请正排表的内存空间 */
    index_dict_table = (index_dict_t *)malloc(sizeof(index_dict_t) * gconfig.max_dict_table_size);
    if (NULL == index_dict_table)
    {
        fprintf(stderr, "Can NOT malloc memory for index dict, need size: %ld\n",
            sizeof(index_dict_t) * gconfig.max_dict_table_size);
        ret = -1;
        goto FINISH;
    }
    for (i = 0; i < gconfig.max_dict_table_size; i++)
    {
        index_dict_table[i].query = (char *)malloc(sizeof(char) * QUERY_LEN);
        if (NULL == index_dict_table[i].query)
        {
            fprintf(stderr, "Can NOT malloc memory for index_dict_table[%d].query, need size: %ld\n",
                i, sizeof(char) * QUERY_LEN);
            ret = -1;
            goto FINISH;
        }

        index_dict_table[i].brief = (char *)malloc(sizeof(char) * BRIEF_LEN);
        if (NULL == index_dict_table[i].brief)
        {
            fprintf(stderr, "Can NOT malloc memory for index_dict_table[%d].brief, need size: %ld\n",
                i, sizeof(char) * BRIEF_LEN);
            ret = -1;
            goto FINISH;
        }
    }

    /**
     * 申请工作进程的内存池,避免动态内存分配
     * 但如果并发量上来,不加锁有可能触发数据错乱
     * 加锁则影响性能,开到足够大能降低概率,不能免除
     * 我的理解 ^_*
     * */

FINISH:
    return ret;
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
            break;

        case 'x':   /** index dict */
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

