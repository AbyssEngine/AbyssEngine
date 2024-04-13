#ifndef ABYSS_SPRITE_H
#define ABYSS_SPRITE_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

struct SpriteFrame {
    SDL_Texture *texture;
    uint32_t     width;
    uint32_t     height;
    int32_t      offset_x;
    int32_t      offset_y;
};

struct Sprite {
    struct SpriteFrame *frames;
    uint32_t            frame_count;
    double              play_length;
    uint32_t            ticks_per_frame;
    uint64_t            last_ticks;
    uint32_t            animation_index;
};

struct Sprite *sprite_load(const char *path, const char *palette_name);
void           sprite_free(struct Sprite *Sprite);
void           sprite_set_blend_mode(const struct Sprite *Sprite, SDL_BlendMode blend_mode);
void           sprite_set_play_length(struct Sprite *Sprite, double play_length);
void           sprite_draw(const struct Sprite *Sprite, uint8_t frame_index, int x, int y);
void sprite_draw_multi(const struct Sprite *Sprite, uint8_t frame_index, int x, int y, int frames_x, int frames_y);
void sprite_draw_animated(struct Sprite *Sprite, int x, int y);
void sprite_load_dc6(struct Sprite *Sprite, const char *path, const char *palette_name);

#endif // ABYSS_SPRITE_H
