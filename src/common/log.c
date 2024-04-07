#include "log.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

log_level_t log_level = LOG_LEVEL_ERROR;

static const char *log_level_strings[] = {
        "",
        "DEBUG",
        "INFO ",
        "WARN ",
        "ERROR",
        "FATAL"
};

void log_set_level(log_level_t level) {
    log_level = level;
}

void log_message(log_level_t level, const char *file, int line, const char *format, ...) {
    va_list args;

    if (level < log_level) {
        return;
    }

    printf("[%s] %s:%i - ", log_level_strings[level], strrchr(file, '/') + 1, line);

    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");

    if (level == LOG_LEVEL_FATAL) {
        char *msg = malloc(sizeof(char) * 10000);
        memset(msg, 0, sizeof(char) * 10000);
        va_start(args, format);
        vsnprintf(msg, 10000, format, args);
        va_end(args);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", msg, sdl_window);
        free(msg);
        exit(-1);
    }
}
