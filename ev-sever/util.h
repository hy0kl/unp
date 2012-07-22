#ifndef _UTIL_H_
#define _UTIL_H_

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

#endif
