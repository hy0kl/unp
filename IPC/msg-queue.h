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
#define BUFFER_SIZE 512
#define ALARMSERVERMSGFILE "/tmp/testMsg.msg"

struct message
{
    long msg_type;
    int  msg_text;
    int  test;
    char msg_body[BUFFER_SIZE];
};

