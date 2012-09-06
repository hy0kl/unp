#include "msg-queue.h"

/**
 * gcc msg-queue.revc.c -o revc
 * */

int main(int argc, char *argv[])
{
    int   qid = 0;
    key_t key;
    message_t msg;
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
    if (-1 == (qid = msgget(key, IPC_CREAT | 0666)))    // 创建
    {
        perror("[Msgget fail at revc]");
        exit(1);
    }

    printf("---revc---\n");
    printf("[Open queue: %d]\n", qid);

    while (1)
    {
        //memset(msg.msg_text, 0, BUFFER_SIZE);
        if (msgrcv(qid, (void*)&msg, sizeof(msg), 0, 0) < 0)    // 接收消息
        {
            perror("[msgrcv]");
            exit(1);
        }

        printf("[The message from process] msg_type: %ld,  msg_test: %d, body: %s",
            msg.msg_type, msg.msg_test, msg.msg_body);

        /**
        if (msg.call_back)
        {
            printf("[msg->call_back] ");
            msg.call_back(&msg);
        }
        else
        {
            fprintf(stderr, "Can NOT invoke call_back function.\n");
        }
        */

        if (0 == strncmp(msg.msg_body, "quit", 4))
        {
            if ((msgctl(qid, IPC_RMID, NULL)) < 0)//从系统内核移走消息队列
            {
                perror("[msgctl fail]");
                exit(1);
            }

            break;
        }
    }

    return 0;
}

