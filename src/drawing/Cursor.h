#ifndef ABYSS_CURSOR_H
#define ABYSS_CURSOR_H

#include <stdbool.h>

enum CursorType { CURSOR_STANDARD, CURSOR_PRESSED };

void cursor_initialize(void);
void cursor_finalize(void);
void cursor_draw(void);
void cursor_set_type(enum CursorType cursor_type);
void cursor_set_visible(bool visible);

#endif // ABYSS_CURSOR_H
