#include "ev_sever.h"

#define PACKAGE "libevent-http-server"
#define VERSION "0.1.0"
#define CRLF    "<br />"
#define FORMAT_HTML "html"
#define FORMAT_JSON "json"

#define _DEBUG  1
#define DAEMON  1

#define DEFAULT_LINK_LENGTH 1024

/**
 * #define logprintf(format, arg...) fprintf(stderr, "%s:%d:%s "format"\n", __FILE__, __LINE__, __func__, ##arg)
 */
#define logprintf(format, arg...) fprintf(stderr, "[NOTIC] [%s] "format"\n", __func__, ##arg)

#define HTTP_TIMEOUT    5
#define DEFAULT_LISTEN  "0.0.0.0"
#define DEFAULT_PORT    8080
#define DEFAULT_INVERTED_INDEX  "./data/inverted_index"
#define DEFAULT_INDEX_DICT      "./data/index_dict"

#define SIGNO_END 11111

#define GETUTIME(t) ((t.tv_sec) * 1000000 + (t.tv_usec))
#define GETSTIME(t) (t.tv_sec)
#define GETMTIME(t) ((((t.tv_sec) * 1000000 + (t.tv_usec))) / 1000)

#define HOST_NAME_LEN   32
#define FILE_NAME_LEN   128
#define QUERY_EXT_SIZE  3

/** output format */
#define OUTPUT_AS_HTML 0
#define OUTPUT_AS_JSON 1

#define TMP_STR_BUF_LEN     1024 * 20

/** about index query */
#define SINGLE_INDEX_SIZE   20
#define MAX_HASH_TABLE_SIZE 49999
#define MAX_DICT_TABLE_SIZE 10000
#define SEARCH_BUF_SIZE     64
#define QUERY_LEN   512
#define BRIEF_LEN   1024 * 10
#define TPL_BUF_LEN 1024 * 80
#define READ_LINE_BUF_LEN   1024 * 10
#define SEPARATOR   "\t"

typedef struct _config_t
{
    int do_daemonize;
    //char *config_file;
    int  port;
    int  timeout;
    int  log_level;
    size_t  max_hash_table_size;
    size_t  max_dict_table_size;
    size_t  search_buf_size;
    char hostname[HOST_NAME_LEN];
    char inverted_index[FILE_NAME_LEN];
    char index_dict[FILE_NAME_LEN];
} config_t;

typedef unsigned long int indext_t;
/** 倒排表单条数据 */
typedef struct _index_item_t
{
    short     size;   /** count(index_chain) */
    indext_t *index_chain;  /** 单个索引链,目前只取 TOP20 */
} index_item_t;

/** hash table list, hash 表拉链,解决冲突 */
typedef struct _hash_list_t
{
    index_item_t        *index_item;
    struct _hash_list_t *next;
} hash_list_t;

typedef struct _query_ext_t
{
    short count;
    /**
     * 最多为 3 组数据
     * 0: 原始 query 词
     * 1: 全拼
     * 2: 简拼
     * */
    char **queries;
} query_ext_t;

typedef struct _dict_data_t
{
    query_ext_t *query_ext;
    char *brief;
} dict_data_t;

/** 正排表单条数据 */
typedef struct _index_dict_t
{
    /** use malloc() */
    // query_ext_t *query_ext;
    // char *brief;
    dict_data_t *dict_data;
} index_dict_t;

typedef struct _dict_ext_t
{
    /** use malloc() */
    char *query;
    char *brief;
} dict_ext_t;

typedef struct _work_buf_t
{
    size_t       array_count;  /** count(dict_data) */
    dict_ext_t  *dict_data;
} work_buf_t;

typedef struct _search_buf_t
{
    size_t current;
    work_buf_t    *work_buf;
    char         **tpl_buf;
} search_buf_t;

/** sever action */
enum s_action_t
{
    ACTION_NONE,
    ACTION_SEARCH,
    ACTION_UPDATE,
    ACTION_DELETE,
};

/**
 * global
 * */
extern config_t      gconfig;
extern hash_list_t  *index_hash_table;  /** 倒排表,开放拉链表  */
extern index_dict_t *index_dict_table;  /** 正排表 */
extern search_buf_t  search_buf;

