#ifndef ABYSS_SPRITE_H
#define ABYSS_SPRITE_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

struct sprite_frame {
    SDL_Texture *texture;
    uint32_t     width;
    uint32_t     height;
    int32_t      offset_x;
    int32_t      offset_y;
};

struct sprite {
    struct sprite_frame *frames;
    uint32_t             frame_count;
    float                play_length;
    uint32_t             ticks_per_frame;
    uint64_t             last_ticks;
    uint32_t             animation_index;
};

struct sprite *sprite_load(const char *path, const char *palette_name);
void           sprite_free(struct sprite *sprite);
void           sprite_set_blend_mode(const struct sprite *sprite, SDL_BlendMode blend_mode);
void           sprite_set_play_length(struct sprite *sprite, float play_length);
void           sprite_draw(const struct sprite *sprite, uint8_t frame_index, int x, int y);
void sprite_draw_multi(const struct sprite *sprite, uint8_t frame_index, int x, int y, int frames_x, int frames_y);
void sprite_draw_animated(struct sprite *sprite, int x, int y);
void sprite_load_dc6(struct sprite *sprite, const char *path, const char *palette_name);

#endif // ABYSS_SPRITE_H
