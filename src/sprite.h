#ifndef ABYSS_SPRITE_H
#define ABYSS_SPRITE_H

#include <stdint.h>
#include <SDL2/SDL.h>

typedef struct sprite_frame_s {
    SDL_Texture*    texture;
    uint32_t        width;
    uint32_t        height;
    int32_t         offset_x;
    int32_t         offset_y;
} sprite_frame_t;

typedef struct sprite_s {
    sprite_frame_t* frames;
    uint32_t        frame_count;
} sprite_t;


sprite_t*   sprite_load         (const char* path, const char* palette_name);
void        sprite_free         (sprite_t* sprite);
void        sprite_draw         (sprite_t* sprite, uint8_t frame_index, int x, int y);
void        sprite_draw_multi   (sprite_t* sprite, uint8_t frame_index, int x, int y, int frames_x, int frames_y);

void        sprite_load_dc6     (sprite_t* sprite, const char* path, const char* palette_name);

#endif // ABYSS_SPRITE_H
