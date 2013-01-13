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
	LOG_LEVEL_DEBUG2,
	LOG_LEVEL_ALL
} loglevel_t;

extern loglevel_t loglevel;
extern FILE *output;

void setLogLevel(loglevel_t new_log);
void setOutputType(FILE *type);

#define CRITICAL(__msg,...) \
	if ( loglevel >= LOG_LEVEL_CRITICAL ){ \
		fprintf(output, "CRITICAL: "); \
		fprintf(output, __msg, ##__VA_ARGS__); \
	}

#define ERROR(__msg,...) \
	if ( loglevel >= LOG_LEVEL_ERROR ){ \
		fprintf(output, "ERROR: "); \
		fprintf(output, __msg, ##__VA_ARGS__); \
	}

#define NOTICE(__msg,...) \
	if ( loglevel >= LOG_LEVEL_NOTICE ){ \
		fprintf(output, "NOTICE: "); \
		fprintf(output, __msg, ##__VA_ARGS__); \
	}

#define DEBUG(__msg,...) \
	if ( loglevel >= LOG_LEVEL_DEBUG ){ \
		fprintf(output, "DEBUG: "); \
		fprintf(output, __msg, ##__VA_ARGS__); \
	}

#define DEBUG2(__msg,...) \
	if ( loglevel >= LOG_LEVEL_DEBUG2 ){ \
		fprintf(output, "DEBUG2: "); \
		fprintf(output, __msg, ##__VA_ARGS__); \
	}

#endif /* _LOGGER_H_ */
