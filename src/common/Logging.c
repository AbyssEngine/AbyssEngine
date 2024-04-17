#include "Logging.h"
#include "Globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

#define MAX_LOG_LINE_LENGTH 4096
enum LogLevel log_level = LOG_LEVEL_ERROR;

static const char *log_level_strings[] = {"", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL"};

void log_set_level(const enum LogLevel level) { log_level = level; }

void log_message(enum LogLevel level, const char *file, int line, const char *format, ...) {
    char    msg[MAX_LOG_LINE_LENGTH];
    va_list args;

    if (level < log_level) {
        return;
    }

#ifdef _WIN32
    printf("[%s] %s:%i - ", log_level_strings[level], strrchr(file, '\\') + 1, line);
    if (IsDebuggerPresent()) {
        memset(msg, 0, sizeof(char) * MAX_LOG_LINE_LENGTH);
        snprintf(msg, MAX_LOG_LINE_LENGTH, "%s(%i): [%s] ", file, line, log_level_strings[level]);
        va_start(args, format);
        const int len = strchr(msg, '\0') - msg;
        vsnprintf(strchr(msg, '\0'), len, format, args);
        va_end(args);
        strcat(msg, "\n");
        OutputDebugString(msg);
    }
#else
    printf("[%s] %s:%i - ", log_level_strings[level], strrchr(file, '/') + 1, line);
#endif
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");

    if (level == LOG_LEVEL_FATAL) {
        memset(msg, 0, sizeof(char) * MAX_LOG_LINE_LENGTH);
        va_start(args, format);
        vsnprintf(msg, MAX_LOG_LINE_LENGTH, format, args);
        va_end(args);

        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", msg, sdl_window);
        exit(-1);
    }
}
