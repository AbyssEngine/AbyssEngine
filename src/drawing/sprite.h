#ifndef ABYSS_SPRITE_H
#define ABYSS_SPRITE_H

#include "SDL2/SDL.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct sprite_frame_s {
    SDL_Texture *texture;
    uint32_t     width;
    uint32_t     height;
    int32_t      offset_x;
    int32_t      offset_y;
} sprite_frame_t;

typedef struct sprite_s {
    sprite_frame_t *frames;
    uint32_t        frame_count;
    float           play_length;
    uint32_t        ticks_per_frame;
    uint16_t        last_ticks;
    uint32_t        animation_index;
} sprite_t;

sprite_t *sprite_load(const char *path, const char *palette_name);
void      sprite_free(sprite_t *sprite);
void      sprite_set_blend_mode(sprite_t *sprite, SDL_BlendMode blend_mode);
void      sprite_set_play_length(sprite_t *sprite, float play_length);
void      sprite_draw(sprite_t *sprite, uint8_t frame_index, int x, int y);
void      sprite_draw_multi(sprite_t *sprite, uint8_t frame_index, int x, int y, int frames_x, int frames_y);
void      sprite_draw_animated(sprite_t *sprite, int x, int y);
void      sprite_load_dc6(sprite_t *sprite, const char *path, const char *palette_name);

#endif // ABYSS_SPRITE_H
