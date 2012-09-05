#include "msg-queue.h"

/**
 * gcc msg-queue.send.c -o send
 * */

int main(int argc, char *argv[])
{
    int qid;
    key_t key;
    struct message msg;
    msg.msg_text = 10;
    //ftok根据不同路径和关键字产生标准的KEY
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
        printf("[Enter some message to the queue]: ");
        if ((fgets(msg.msg_body, BUFFER_SIZE, stdin)) == NULL)
        {
            fprintf(stderr, "[No message]");
            exit(1);
        }

        msg.msg_type = getpid();//消息类型为进程号
        msg.test     = rand() % 100;
        
        if((msgsnd(qid, &msg, sizeof(msg), 0)) < 0)//发送消息
        {
          perror("message posted");
          exit(1);
        }
        
        if (0 == strncmp(msg.msg_body, "quit", 4))
        {
            break;
        }
    }

    return 0;
}

