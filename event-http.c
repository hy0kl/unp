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

    /* 分析URL参数 */
    char *decode_uri = strdup((char*) evhttp_request_uri(req));
    struct evkeyvalq http_query;
    evhttp_parse_query(decode_uri, &http_query);
    free(decode_uri);

    //接收GET表单参数name
    const char *http_input_name = evhttp_find_header(&http_query, "name");

    //处理输出header头
    evhttp_add_header(req->output_headers, "Content-Type", "text/html");
    evhttp_add_header(req->output_headers, "Connection", "keep-alive");
    evhttp_add_header(req->output_headers, "Cache-Control", "no-cache");

    //处理输出数据
    evbuffer_add_printf(buf, "<html><body><head>Libevent Http Sever</head><body>");
    evbuffer_add_printf(buf, "PROXY VERSION %s%s\n", VERSION, CRLF);
    evbuffer_add_printf(buf, "------------------------------%s\n", CRLF);
    evbuffer_add_printf(buf, "YOU PASS NAME: %s%s\n", http_input_name, CRLF);
    evbuffer_add_printf(buf, "</body></html>");

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

