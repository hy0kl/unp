#include "msg-queue.h"

/**
 * gcc msg-queue.revc.c -o revc
 * */

int main(int argc, char *argv[])
{
    int qid;
    key_t key;
    struct message msg;
    char msgfile[512];
    // time_t t = time(NULL);
    strcpy(msgfile, ALARMSERVERMSGFILE);
    int fid = open(msgfile, O_RDWR | O_CREAT, FILE_MODE);
    if (-1 == fid)
    {
            return -1;
    }
    close(fid);
    
    key = ftok(msgfile, 'a');
    if((qid = msgget(key, IPC_CREAT | 0666)) == -1)//创建
    {
        perror("msgget");
        exit(1);
    }
    
    printf("open queue %d\n", qid);
    while (1)
    {
        //memset(msg.msg_text, 0, BUFFER_SIZE);
        if (msgrcv(qid, (void*)&msg, sizeof(msg), 0, 0) < 0)//接收消息
        {
            perror("msgrcv");
            exit(1);
        }
        printf("[The message from process] msg_type: %ld,  test: %d, body: %s",
            msg.msg_type, msg.test, msg.msg_body);
        
        if (0 == strncmp(msg.msg_body, "quit", 4))
        {
            if((msgctl(qid, IPC_RMID, NULL)) < 0)//从系统内核移走消息队列
            {
                perror("msgctl");
                exit(1);
            }

            break;
        }
    }

    return 0;
}

