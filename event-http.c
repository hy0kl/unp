#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <err.h>
#include <event.h>
#include <evhttp.h>

#define VERSION "1.0"
#define CRLF    "<br />"
#define FORMAT_HTML "html"
#define FORMAT_JSON "json"

/**
 * #define logprintf(format, arg...) fprintf(stderr, "%s:%d:%s "format"\n", __FILE__, __LINE__, __func__, ##arg)
 */
#define logprintf(format, arg...) fprintf(stderr, "[NOTIC] [%s] "format"\n", __func__, ##arg)

/*
 * 处理模块
 * gw event-http.c -o http-sever -levent
 * http://localhost:8012/?name=test
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
    char callback[16] = {0};
    int callback_validate = 0;

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
    evhttp_add_header(req->output_headers, "Connection", "keep-alive");
    evhttp_add_header(req->output_headers, "Cache-Control", "no-cache");

    //处理输出数据
    if (output_format)
    {
        evbuffer_add_printf(buf, "%s{\"stat\": 200,\
\"info\": {\"notice\": \"welcome to libevent word.\",\
\"version\": \"%s\"\
}\
}%s", callback_validate ? callback : "", VERSION,
        callback_validate ? ")" : "");
    }
    else
    {
        evbuffer_add_printf(buf, "<html><body><head>\
<title>Libevent Http Sever</title>\
</head><body>");
        evbuffer_add_printf(buf, "PROXY VERSION %s%s\n", VERSION, CRLF);
        evbuffer_add_printf(buf, "------------------------------%s\n", CRLF);
        evbuffer_add_printf(buf, "YOU PASS name: %s%s\n", http_input_name ? http_input_name : "NONE", CRLF);
        evbuffer_add_printf(buf, "</body></html>");
    }

    //返回code 200
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    //释放内存
    evhttp_clear_headers(&http_query);
    evbuffer_free(buf);
}

int main(int argc, char** argv)
{
    struct evhttp *httpd;
    char *proxy_listen = "0.0.0.0";//绑定所有ip
    int proxy_port = 8012;//端口号
    int proxy_settings_timeout = 5; //http请求超时时间

    //初始化监听ip和端口
    event_init();
    httpd = evhttp_start(proxy_listen, proxy_port);
    if (httpd == NULL)
    {
        fprintf(stderr, "Error: Unable to listen on %s:%d\n\n", proxy_listen, proxy_port);
        exit(1);
    }

    //设置http连接超时时间
    evhttp_set_timeout(httpd, proxy_settings_timeout);
    //设置请求到达后的回调函数
    evhttp_set_gencb(httpd, api_proxy_handler, NULL);
    //libevent循环处理事件
    event_dispatch();
    //释放资源
    evhttp_free(httpd);

    return 0;
}
