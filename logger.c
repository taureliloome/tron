
#include "logger.h"

loglevel_t loglevel = LOG_LEVEL_DEBUG;
FILE *output = NULL;

void setLogLevel(loglevel_t new_log) {
	loglevel = new_log;
}

void setOutputType(FILE *type) {
	output = type;
}

