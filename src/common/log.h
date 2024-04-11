#ifndef ABYSS_LOG_H
#define ABYSS_LOG_H

#include <stdarg.h>

#define LOG_DEBUG(...) log_message(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...)  log_message(LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  log_message(LOG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log_message(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...)                                                                                                 \
    log_message(LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__);                                                     \
    exit(-1)

#define FAIL_IF_NULL(ptr)                                                                                              \
    if (ptr == NULL) {                                                                                                 \
        LOG_FATAL("Failed to allocate memory.");                                                                       \
    }

enum log_level {
    LOG_LEVEL_EVERYTHING,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
};

extern enum log_level log_level;

void log_set_level(enum log_level level);
void log_message(enum log_level level, const char *file, int line, const char *format, ...);

#endif // ABYSS_LOG_H
