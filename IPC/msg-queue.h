#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/file.h>

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)
#define BUFFER_SIZE     512
#define FILE_NAME_LEN   1024
#define ALARMSERVERMSGFILE "/tmp/testMsg.msg"

typedef struct _message_t
{
    long msg_type;
    int  msg_test;
    char msg_body[BUFFER_SIZE];
    //void (*call_back)(struct _message_t *msg);
} message_t;

void print_type(message_t *msg)
{
    printf("[Message type]: %ld\n", msg->msg_type);
    return;
}


