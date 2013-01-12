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
static FILE *output = NULL;

void setLogLevel(loglevel_t new_log) {
	loglevel = new_log;
}

void setOutputType(FILE *type) {
	output = type;
}

#define CRITICAL(__msg,...) \
	if ( loglevel >= LOG_LEVEL_CRITICAL ){ \
		fprintf(output, "DEBUG: "); \
		fprintf(output, __msg, ##__VA_ARGS__); \
	}

#define ERROR(__msg,...) \
	if ( loglevel >= LOG_LEVEL_ERROR ){ \
		fprintf(output, "DEBUG: "); \
		fprintf(output, __msg, ##__VA_ARGS__); \
	}

#define NOTICE(__msg,...) \
	if ( loglevel >= LOG_LEVEL_NOTICE ){ \
		fprintf(output, "DEBUG: "); \
		fprintf(output, __msg, ##__VA_ARGS__); \
	}

#define DEBUG(__msg,...) \
	if ( loglevel >= LOG_LEVEL_DEBUG ){ \
		fprintf(output, "DEBUG: "); \
		fprintf(output, __msg, ##__VA_ARGS__); \
	}

#endif /* _LOGGER_H_ */
