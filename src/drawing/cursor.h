#ifndef ABYSS_CURSOR_H
#define ABYSS_CURSOR_H

#include <stdbool.h>

typedef enum {
    CURSOR_STANDARD,
    CURSOR_PRESSED
} cursor_type_t;


void cursor_initialize();

void cursor_finalize();

void cursor_draw();

void cursor_set_type(cursor_type_t cursor_type);

void cursor_set_visible(bool visible);

#endif // ABYSS_CURSOR_H
