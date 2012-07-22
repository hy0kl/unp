#include "ev_sever.h"
#include "util.h"

#define VERSION "1.0"
#define CRLF    "<br />"
#define FORMAT_HTML "html"
#define FORMAT_JSON "json"

#define DAEMON  1

/**
 * #define logprintf(format, arg...) fprintf(stderr, "%s:%d:%s "format"\n", __FILE__, __LINE__, __func__, ##arg)
 */
#define logprintf(format, arg...) fprintf(stderr, "[NOTIC] [%s] "format"\n", __func__, ##arg)

#define HTTP_TIMEOUT    5
#define DEFAULT_LISTEN  "0.0.0.0"
#define DEFAULT_PORT    8080
