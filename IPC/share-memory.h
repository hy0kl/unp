#define FILE_NAME_LEN   512
//#define SHM_FILE_NAME   "/dev/shm/myshm2"
#define SHM_FILE_NAME   "/tmp/shm"
#define SHM_ITEM_NUM    11
#define CACHE_BUF_LEN   1024 * 4

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)

typedef struct _shm_item_t
{
    char cache[CACHE_BUF_LEN];
    int  shm_id;
} shm_item_t;

