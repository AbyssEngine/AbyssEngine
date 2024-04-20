#ifndef ABYSS_CURSOR_H
#define ABYSS_CURSOR_H

#include <stdbool.h>

enum CursorType { CURSOR_STANDARD, CURSOR_PRESSED };

void Cursor_CreateSingleton(void);
void Cursor_Destroy(void);
void Cursor_Draw(void);
void Cursor_SetType(enum CursorType cursor_type);
void Cursor_SetVisible(bool visible);

#endif // ABYSS_CURSOR_H
