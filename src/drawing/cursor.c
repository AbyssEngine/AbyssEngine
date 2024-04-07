#include "cursor.h"
#include "../common/globals.h"
#include "sprite.h"

#define CURSOR_FRAME_NORMAL     0
#define CURSOR_FRAME_PRESSED    5
#define CURSOR_FRAME_HAND       6

sprite_t *cursor_sprite;
cursor_type_t cursor_sprite_type;
bool cursor_visible;

void cursor_initialize() {
    cursor_sprite = sprite_load(CURSOR_DEFAULT, PALETTE_UNITS);
    cursor_visible = true;
    cursor_set_type(CURSOR_STANDARD);

    sprite_set_blend_mode(cursor_sprite, SDL_BLENDMODE_BLEND);
}

void cursor_finalize() {
    sprite_free(cursor_sprite);
}

void cursor_draw() {
    if (!cursor_visible) {
        return;
    }
    sprite_draw(cursor_sprite, cursor_sprite_type, mouse_x, mouse_y + 2);
}

void cursor_set_type(cursor_type_t cursor_type) {
    cursor_sprite_type = cursor_type;
}

void cursor_set_visible(bool visible) {
    cursor_visible = visible;
}
