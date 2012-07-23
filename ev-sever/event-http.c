#include "event-http.h"

/*
 * 处理模块
 * gw event-http.c -o http-sever -levent
 * http://localhost:DEFAULT_PORT/?name=test
 * TODO: add getopt() and gconf.
 * TODO: add lua to create JSON data.
 * TODO: add log logic.
*/
void api_proxy_handler(struct evhttp_request *req, void *arg)
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

/**
 * global variables
 * */
config_t gconfig;

static void init_config()
{
    gconfig.do_daemonize = 1;
    gconfig.hostname[0]  = '\0';
    gconfig.port         = 0;
    gconfig.timeout      = 0;
    gconfig.log_level    = 0;
    gconfig.dict_file[0] = '\0';

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
    while (-1 != (c = getopt(argc, argv,
        "d:"  /* work as daemon process */
        /*"c:"   config file name  */
        "H:"  /* http hostname -i */
        "p:"  /* http listen port  */
        "t:"  /* http timeout */
        "v:"  /* show version */
        "l:"  /* log level */
        "i:"  /* input dict file name */
        "h:"  /* show usage */
    )))
    {
        switch (c)
        {
        case 'd':
            gconfig.do_daemonize = atoi(optarg);
            break;
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

