#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>
#include <stdlib.h>

typedef enum loglevel_e{
	LOG_LEVEL_NONE,
	LOG_LEVEL_CRITICAL,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_NOTICE,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_ALL
} loglevel_t;
static loglevel_t loglevel = LOG_LEVEL_DEBUG;

void setLogLevel(loglevel_t new_log) {
	loglevel = new_log;
}

#define CRITICAL(__msg,...) \
	if ( loglevel >= LOG_LEVEL_CRITICAL ){ \
		fprintf(stderr, "DEBUG: "); \
		fprintf(stderr, __msg, ##__VA_ARGS__); \
	}

#define ERROR(__msg,...) \
	if ( loglevel >= LOG_LEVEL_ERROR ){ \
		fprintf(stderr, "DEBUG: "); \
		fprintf(stderr, __msg, ##__VA_ARGS__); \
	}

#define NOTICE(__msg,...) \
	if ( loglevel >= LOG_LEVEL_NOTICE ){ \
		fprintf(stderr, "DEBUG: "); \
		fprintf(stderr, __msg, ##__VA_ARGS__); \
	}

#define DEBUG(__msg,...) \
	if ( loglevel >= LOG_LEVEL_DEBUG ){ \
		fprintf(stderr, "DEBUG: "); \
		fprintf(stderr, __msg, ##__VA_ARGS__); \
	}

#endif /* _LOGGER_H_ */
