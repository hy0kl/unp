#include "event-http.h"

/**
 * global variables
 * */
config_t gconfig;
index_term_t *index_hash_table;

/*
 * 处理模块
 * gw event-http.c -o http-sever -levent
 * http://localhost:DEFAULT_PORT/?name=test
 * TODO: add getopt() and gconf.
 * TODO: add lua to create JSON data.
 * TODO: add log logic.
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

int main(int argc, char** argv)
{
    int c;
    struct evhttp *httpd = NULL;
    char *proxy_listen   = DEFAULT_LISTEN;    //绑定所有ip
    int proxy_port       = DEFAULT_PORT;      //端口号
    int proxy_settings_timeout = HTTP_TIMEOUT;     //http请求超时时间

    signal_setup();

    init_config();
    /* process arguments */
    /*"c:"   config file name  */
    while (-1 != (c = getopt(argc, argv,
        "d:"  /* work as daemon process */
        "s:"  /* max hash table size */
        "a:"  /* max dict table size */
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
            gconfig.do_daemonize = atoi(optarg);
            break;

        case 'v':
        case 'h':
            usage();
            exit(EXIT_SUCCESS);
        }
    }

#if (DAEMON)
    if (-1 == daemonize(0, 1))
    {
        fprintf(stderr, "failed to daemon() in order to daemonize\n");
        exit(EXIT_FAILURE);
    }
#endif

    /** 初始化事件 */
    event_init();
    /** 初始化监听ip和端口 */
    httpd = evhttp_start(proxy_listen, proxy_port);
    if (NULL == httpd)
    {
        fprintf(stderr, "[Error]: Unable to listen on %s:%d\n", proxy_listen, proxy_port);
        exit(1);
    }

    // 设置http连接超时时间
    evhttp_set_timeout(httpd, proxy_settings_timeout);
    // 设置请求到达后的回调函数
    evhttp_set_gencb(httpd, api_proxy_handler, NULL);
    // libevent 循环处理事件
    event_dispatch();
    // 释放资源
    evhttp_free(httpd);

    return 0;
}

