/**
 * c multiple thread programming in linux
 * author : Jerry Yang
 * E-mail : hy0kle@gmail.com
 * gcc -lpthread thread.c -o thread
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

/**
 * 工作者线程线
 * */
#define MAX 16

typedef struct _task_queue_t
{
    int id;
    struct _task_queue_t *next;
} task_queue_t;

typedef struct _argument_t
{
    int tindex;
} argument_t;

int task_id = 0;
int g_completed_flag = 0;
task_queue_t    *task_queue_head = NULL;
task_queue_t    *task_queue_tail = NULL;
pthread_mutex_t  task_queue_mutex;

void *server_run(void *arg)
{
    size_t size = 0;
    task_queue_t *tmp_task = NULL;

    while (1)
    {
        pthread_mutex_lock(&task_queue_mutex);

        size = sizeof(task_queue_t);
        tmp_task = (task_queue_t *)malloc(size);
        if (NULL == tmp_task)
        {
            fprintf(stdout, "Can NOT malloc memory for tmp_task, need size: %lu\n", size);
            exit(-1);
        }
        tmp_task->id   = task_id;
        tmp_task->next = NULL;

        if (NULL == task_queue_head)
        {
            task_queue_head = task_queue_tail = tmp_task;
        }
        else
        {
            task_queue_tail->next = tmp_task;
            task_queue_tail = tmp_task;
        }

        fprintf(stdout, "Current task_id: %d\n", task_id);
        task_id++;
        pthread_mutex_unlock(&task_queue_mutex);

        usleep((useconds_t)(5000));
        //sleep(1);

        if (task_id > 31)
        {
            g_completed_flag = 1;
            break;
        }
    }

    return NULL;
}

void *server_core(void *arg)
{
    argument_t *thread_arg = (argument_t *)arg;
    const int tindex = thread_arg->tindex;
    task_queue_t *tmp_task = NULL;

    while (1)
    {
        if (g_completed_flag && NULL == task_queue_head)
        {
            fprintf(stdout, "  [%d]My job has done.\n", tindex);
            break;
        }

        fprintf(stdout, "I am worker: %d\n", tindex);

        pthread_mutex_lock(&task_queue_mutex);
        if (NULL == task_queue_head)
        {
            usleep((useconds_t)(5000));
            //sleep(2);
            pthread_mutex_unlock(&task_queue_mutex);
            continue;
        }
        tmp_task = task_queue_head;
        task_queue_head = task_queue_head->next;
        pthread_mutex_unlock(&task_queue_mutex);

        fprintf(stdout, "  [%d] I handle task: %d\n", tindex, tmp_task->id);
        free(tmp_task);
     }

    return NULL;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    int i = 0;
    argument_t arg[MAX];

    pthread_t *pt_server_core, pt_server_run;

    pthread_mutex_init(&task_queue_mutex, NULL);

    // 分配工作线程空间
    pt_server_core = (pthread_t *) calloc(MAX, sizeof(pthread_t));
    fprintf(stdout, "malloc threadid for work threads, thread num is %d\n", MAX);
    if (! pt_server_core)
    {
        fprintf(stdout, "malloc threads ids for work thrads fail, exit.\n");
        exit(-1);
    }

    ret = pthread_create(&(pt_server_run), NULL, server_run, NULL);
    if ( 0 != ret )
    {
        fprintf(stdout, "Create server_run thread fail, exit.");
        exit (-1);
    }

    /** 创建每个工作者 */
    for (i = 0; i < MAX; ++i)
    {
        arg[i].tindex = i;
        ret = pthread_create(&pt_server_core[i], NULL, server_core, (void *)&(arg[i]));
        if ( 0 != ret )
        {
            fprintf(stdout, "create the %dth server_core thread fail, exit.\n", i);
            exit (-1);
        }
        fprintf(stdout, "create the %dth server_core thread success, thread id is %lu.\n",
                i, (unsigned long int)pt_server_core[i]);
    }

    pthread_join(pt_server_run, NULL);

    for (i = 0; i < MAX; ++i)
    {
        pthread_join(pt_server_core[i], NULL);
    }

    fprintf(stdout, "sizeof(pthread_mutex_t) = %lu\n", sizeof(pthread_mutex_t));

    return 0;
}
