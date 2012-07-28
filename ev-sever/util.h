#ifndef _UTIL_H_
#define _UTIL_H_
#include "event-http.h"
/**
 * get localtime string.
 * */
int get_localtime_str(char *src, const size_t buf_len);

/**
 * set the program as daemon
 * */
int daemonize(int nochdir, int noclose);

/**
 * set up signal
 * */
void signal_setup();

indext_t hash(const char *key, const int hash_table_size);

/**
 * Replace all occurrences of the search string with the replacement string
 * The value being searched for, otherwise known as the needle.
 * The replacement value that replaces found search values.
 * */
char * str_replace(char *src, const size_t buf_size, const char *search, const char *replace);

char *strtolower(char *src, const size_t buf_len, const char *encoding);

#endif

