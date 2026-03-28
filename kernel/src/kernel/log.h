#pragma once

typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
} log_level_t;

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_DEBUG
#endif

void log_init(void);
void log_msg(log_level_t level, const char* subsystem, const char* fmt, ...);

#define log_debug(subsys, ...) log_msg(LOG_DEBUG, subsys, __VA_ARGS__)
#define log_info(subsys, ...) log_msg(LOG_INFO, subsys, __VA_ARGS__)
#define log_warn(subsys, ...) log_msg(LOG_WARN, subsys, __VA_ARGS__)
#define log_error(subsys, ...) log_msg(LOG_ERROR, subsys, __VA_ARGS__)
