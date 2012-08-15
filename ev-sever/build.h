#ifndef _BUILD_H_
#define _BUILD_H_

#include <pthread.h>
#include "util.h"
#include "pinyin_data.h"

#define BUILD_PACKAGE   "build-worker"
#define BUILD_VERSION   "1.0"

#define THREAD_NUM      16
#define DEFAULT_ORIGINAL_FILE   "./data/original"

#define SEPARATOR           "\t"
#define DEFAULT_ENCODING    "utf-8"
#define COMMENT_LINE_FLAG   "#"

#define PIPE_READER     0
#define PIPE_WRITER     1

/**
 * 分三组
 * 1: 本身的前缀
 * 2: 全拼
 * 3: 简拼
 * */
#define MB_LENGTH           8
#define PREFIX_ARRAY_SIZE   3 * MB_LENGTH

#define PREFIX_LEN          64
#define ORIGINAL_LINE_LEN   2048
#define DEFAULT_WEIGHT_ARRAY_SIZE   256
#define LOG_BUF_LEN         1024 * 3

typedef struct _prefix_array_t
{
    short count;
    char  data[PREFIX_ARRAY_SIZE][PREFIX_LEN];
} prefix_array_t;

typedef struct _task_queue_t task_queue_t;
struct _task_queue_t
{
    prefix_array_t  prefix_array;
    char            original_line[ORIGINAL_LINE_LEN];
    indext_t        dict_id;
    task_queue_t   *next;
};

typedef struct _weight_item_t
{
    indext_t dict_id;
    float    weight;
} weight_item_t;

typedef struct _weight_array_t
{
    short count;
    weight_item_t weight_item[DEFAULT_WEIGHT_ARRAY_SIZE];
} weight_array_t;

/** hash table list, hash 表拉链,解决冲突,建立索引时用来去重 */
typedef struct _hash_list_ext_t
{
    char                    *prefix;
    struct _hash_list_ext_t *next;
} hash_list_ext_t;

typedef struct _orig_list_t
{
    char *orig_line;
    struct _orig_list_t *next;
} orig_list_t;

typedef struct _argument_t
{
        int tindex;
} argument_t;

typedef struct _output_fp_t
{
    FILE *inverted_fp;
    FILE *dict_fp;
} output_fp_t;

#endif
