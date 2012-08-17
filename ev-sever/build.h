#ifndef _BUILD_H_
#define _BUILD_H_

#include "util.h"
#include "utf8vector.h"
#include "pinyin.h"

#define BUILD_PACKAGE   "build-worker"
#define BUILD_VERSION   "0.1.0"

#define DEFAULT_ORIGINAL_FILE   "./data/original"

#define SEPARATOR           "\t"
#define DEFAULT_ENCODING    "utf-8"
#define COMMENT_LINE_FLAG   "#"


/**
 * 分三组
 * 1: 本身的前缀
 * 2: 全拼
 * 3: 简拼
 * */
#define MB_LENGTH   16
#define PREFIX_LEN  512
#define PREFIX_ARRAY_SIZE   3 * MB_LENGTH

#define ORIGINAL_LINE_LEN   2048
#define DEFAULT_WEIGHT_ARRAY_SIZE   1024
#define LOG_BUF_LEN         1024 * 10

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
    char *prefix;
    weight_array_t  *weight_array;
    struct _hash_list_ext_t *next;
} hash_list_ext_t;

typedef struct _output_fp_t
{
    FILE *inverted_fp;
    FILE *dict_fp;
} output_fp_t;

typedef struct _prefix_array_t
{
    short count;
    char  prefix[PREFIX_ARRAY_SIZE][PREFIX_LEN];
} prefix_array_t;
#endif
