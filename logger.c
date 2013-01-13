
#include "logger.h"

loglevel_t loglevel = LOG_LEVEL_DEBUG;
FILE *output = NULL;

void setLogLevel(loglevel_t new_log) {
	loglevel = new_log;
	DEBUG("Loglevel changed\n");
}

void setOutputType(FILE *type) {
	output = type;
	DEBUG("Output type changed\n");
}

