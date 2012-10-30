#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <err.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <sys/ipc.h>
#include <sys/shm.h>


#include "share-memory.h"

/**
 * @describe:
 * @author: hy0kle@gmail.com
 * */

int main(int argc, char *argv[])
{
    int shm_id, i;
    key_t key;
    shm_item_t *shm_p = NULL;
    char shm_file[FILE_NAME_LEN];
    char  tmp;

    snprintf(shm_file, sizeof(shm_file), "%s", SHM_FILE_NAME);
    int fid = open(shm_file, O_RDWR | O_CREAT, FILE_MODE);
    if (-1 == fid)
    {
        return -1;
    }
    close(fid);

    key = ftok(shm_file, 'a');
    if (key == -1)
    {
        perror("ftok error");
        exit(-2);
    }

    shm_id = shmget(key, sizeof(shm_item_t) * SHM_ITEM_NUM, IPC_CREAT);
    if (shm_id == -1)
    {
        perror("shmget error");
        exit(-1);
    }

    fprintf(stderr, "Begin to shmat().\n");
    shm_p = (shm_item_t*)shmat(shm_id, NULL, 0);
    tmp = 'a';
    for (i = 0; i < SHM_ITEM_NUM; i++)
    {
        fprintf(stderr, "create %d for shm.\n", i);
        tmp += 1;
        (*(shm_p + i)).shm_id = i;
        snprintf((*(shm_p + i)).cache, sizeof((*(shm_p + i)).cache), "I am is: [%c]\n", tmp);
    }

    if (shmdt(shm_p) == -1)
        perror(" detach error ");

    return 0;
}

