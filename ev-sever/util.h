#ifndef _UTIL_H_
#define _UTIL_H_
#include "event-http.h"
/**
 * get localtime string.
 * */
int get_localtime_str(char *src, size_t buf_len);

/**
 * set the program as daemon
 * */
int daemonize(int nochdir, int noclose);

/**
 * set up signal
 * */
void signal_setup();

indext_t hash_word(const char *key, int hash_table_size);

#endif
