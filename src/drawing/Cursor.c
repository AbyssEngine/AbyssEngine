#include "Cursor.h"
#include "../common/Globals.h"
#include "Sprite.h"

#define CURSOR_FRAME_NORMAL  0
#define CURSOR_FRAME_PRESSED 5
#define CURSOR_FRAME_HAND    6

struct Sprite  *cursor_sprite;
enum CursorType cursor_sprite_type;
bool            cursor_visible;

void Cursor_CreateSingleton(void) {
    cursor_sprite  = Sprite_Create(CURSOR_DEFAULT, PALETTE_UNITS);
    cursor_visible = true;
    Cursor_SetType(CURSOR_STANDARD);

    Sprite_SetBlendMode(cursor_sprite, SDL_BLENDMODE_BLEND);
}

void Cursor_Destroy(void) { Sprite_Destroy(&cursor_sprite); }

void Cursor_Draw(void) {
    if (!cursor_visible) {
        return;
    }
    Sprite_Draw(cursor_sprite, cursor_sprite_type, mouse_x, mouse_y + 2);
}

void Cursor_SetType(enum CursorType cursor_type) { cursor_sprite_type = cursor_type; }

void Cursor_SetVisible(bool visible) { cursor_visible = visible; }
