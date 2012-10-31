#include "msg-queue.h"

/**
 * gcc msg-queue.send.c -o send
 * */

int main(int argc, char *argv[])
{
    int   qid = 0;
    key_t key;
    message_t msg;
    //ftok根据不同路径和关键字产生标准的KEY
    char msgfile[FILE_NAME_LEN];
    // time_t t = time(NULL);
    strcpy(msgfile, ALARMSERVERMSGFILE);

    int fid = open(msgfile, O_RDWR | O_CREAT, FILE_MODE);
    if (-1 == fid)
    {
        return -1;
    }
    close(fid);

    key = ftok(msgfile, 'a');
    if (-1 == (qid = msgget(key, IPC_CREAT | 0666)))    //创建
    {
        perror("msgget error");
        exit(1);
    }
    printf("---send---\n");
    printf("[Open queue: %d]\n", qid);

    while (1)
    {
        printf("[Enter some message to the queue]: ");
        if (NULL == (fgets(msg.msg_body, BUFFER_SIZE, stdin)))
        {
            fprintf(stderr, "[No message]\n");
            exit(1);
        }

        msg.msg_type = getpid();    //消息类型为进程号
        msg.msg_test     = rand() % 100;

        if ((msgsnd(qid, &msg, sizeof(msg), 0)) < 0)    //发送消息
        {
          perror("[Message posted]");
          exit(1);
        }

        /**
        msg.call_back = print_type;
        if (msg.call_back)
        {
            msg.call_back(&msg);
        }
        */

        if (0 == strncmp(msg.msg_body, "quit", 4))
        {
            break;
        }

    }

    return 0;
}

