/*
 * author: dengzhaoqun
 * date: 2011/07/21
 * platform: linux
 */
#include <sys/select.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEFAULT_PAGE "indexx.html"
#define WEB_ROOT "/Users/hy0kl/www"

void connection(int fd);
int recv_new(int fd, char *str);
void send_new(int fd, char *str);
int get_file_size(int fd);

int main()
{
    int ser_fd, cli_fd, max_fd, fd;
    struct sockaddr_in ser_addr;
    struct sockaddr_in cli_addr;
    int ser_addr_len, cli_addr_len;
    fd_set readfds, master;
    int ret;


    ser_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(ser_fd == -1)
    {
        perror("socket failed");
        exit(1);
    }

    ser_addr_len = sizeof(ser_addr);
    memset(&ser_addr, 0, ser_addr_len);

    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    ser_addr.sin_port = htons(8111);

    ret = bind(ser_fd, (struct sockaddr *)&ser_addr, ser_addr_len);
    if(ret == -1)
    {
        perror("bind failed");
        exit(1);
    }

    listen(ser_fd, 5);

    FD_ZERO(&master);
    FD_SET(ser_fd, &master);
    max_fd = ser_fd;

    while(1)
    {
        readfds = master;
        ret = select(max_fd + 1, &readfds, 0, 0, 0);
        if(ret == -1)
        {
            perror("select failed");
        }
        else
        {
            for(fd = 0; fd < max_fd + 1; fd ++)
            {
                if(FD_ISSET(fd, &readfds))
                {
                    if(fd == ser_fd)
                    {
                        cli_addr_len = sizeof(cli_addr);
                        memset(&cli_addr, 0, cli_addr_len);
                        cli_fd = accept(ser_fd,
                                    (struct sockaddr *)&cli_addr,
                                (socklen_t *)&cli_addr_len);
                        if(cli_fd > max_fd)
                        {
                            max_fd = cli_fd;
                        }

                        FD_SET(cli_fd, &master);
                    }
                    else
                    {
                        connection(fd);
                        FD_CLR(fd, &master);
                    }
                }
            }
        }
    }

}/* main() */



void
connection(int fd)
{
    char request[1000], resource[1000], *ptr;
    int ret;
    int page_fd;

    ret = recv_new(fd, request);

    ptr = strstr(request, " HTTP/");
    if(ptr == NULL)
    {
        perror("Not HTTP request");
    }
    else
    {
        *ptr = 0;
        ptr = NULL;

        if(strncmp(request, "GET ", 4) == 0)
        {
            ptr = request + 4;
        }
        if(strncmp(request, "HEAD ", 5) == 0)
        {
            ptr = request + 5;
        }
        if(ptr == NULL)
        {
            perror("Unknown Request. \n");
        }
        else
        {
            memset(resource, 0, sizeof(resource));
            strncpy(resource, WEB_ROOT, sizeof(resource));
            strncat(resource, ptr, sizeof(resource) - sizeof(WEB_ROOT) -1);
            if(ptr[strlen(ptr) -1] == '/' )
            {
                strncat(resource, DEFAULT_PAGE, sizeof(resource)
                        - strlen(resource)
                        - sizeof(DEFAULT_PAGE)
                        - 1);
            }

            if((page_fd = open(resource, O_RDONLY)) == -1)
            {
                fprintf(stderr, "resource: %s\n", resource);
                perror("open failed, file not found");
                send_new(fd, "HTTP/1.1 404 NOT FOUND\r\n\r\n");
                send_new(fd, "<html><head><title>404 NOT FOUND</title></head>");
                send_new(fd, "<body>Sorry, NOT FOUND</body></html>");
                close(fd);
            }
            else
            {
                int file_size;

                memset(request, 0, sizeof(request));
                file_size = get_file_size(page_fd);
                read(page_fd, request, file_size);

                send_new(fd, "HTTP/1.1 200 OK\r\n");
                send_new(fd, "\r\n");
                send_new(fd, request);
                close(fd);
            }
        }
    }
}/* connection() */


void
send_new(int fd, char *str)
{
    int len;
    len = strlen(str);
    write(fd, str, len);
}/* send_new() */

int
get_file_size(int fd)
{
    struct stat buf;

    fstat(fd, &buf);

    return(buf.st_size);
}/* get_file_size() */

int
recv_new(int fd, char *str)
{
    char *p = str;
    int flag;

    flag = 0;
    while(1)
    {
        read(fd, p, 1);
        if(*p == '\r')
            flag = 1;
        if( flag && (*p == '\n'))
        {
            flag = 0;
            break;
        }
        p ++;
    }
    *(p-1) = '\0';

    return(strlen(str));
}/* recv_new() */
