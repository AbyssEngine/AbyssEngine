#include "Sprite.h"
#include "../common/Globals.h"
#include "../common/Logging.h"
#include "../types/DC6.h"
#include "../types/Palette.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char *get_extension(const char *path) {
    const char *ext = strrchr(path, '.') + 1;
    if (strlen(path) <= (size_t)(ext - path)) {
        LOG_FATAL("Invalid path extension for '%s'.", path);
    }

    char *result = strdup(ext);
    for (char *ch = result; *ch; ch++) {
        *ch = (char)tolower((int)*ch);
    }

    return result;
}

struct Sprite *sprite_load(const char *path, const char *palette_name) {
    struct Sprite *result = malloc(sizeof(struct Sprite));
    FAIL_IF_NULL(result);

    memset(result, 0, sizeof(struct Sprite));

    char *file_ext = get_extension(path);

    if (strcmp(file_ext, "DC6") == 0) {
        sprite_load_dc6(result, path, palette_name);
    } else {
        LOG_FATAL("Could not load Sprite: '%s' is an unsupported file extension.", file_ext);
    }

    free(file_ext);
    return result;
}

void sprite_load_dc6(struct Sprite *Sprite, const char *path, const char *palette_name) {
    struct DC6           *DC6     = dc6_load(path);
    const struct Palette *Palette = palette_get(palette_name);

    Sprite->frame_count     = DC6->header.directions * DC6->header.frames_per_direction;
    Sprite->frames          = malloc(sizeof(struct SpriteFrame) * Sprite->frame_count);
    Sprite->last_ticks      = SDL_GetTicks64();
    Sprite->animation_index = 0;

    FAIL_IF_NULL(Sprite->frames);

    sprite_set_play_length(Sprite, 1.0f);

    for (uint32_t i = 0; i < Sprite->frame_count; i++) {
        const struct DC6Frame *dc6_frame = &DC6->frames[i];
        struct SpriteFrame    *spr_frame = &Sprite->frames[i];

        spr_frame->texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC,
                                               (int)dc6_frame->header.width, (int)dc6_frame->header.height);

        spr_frame->width    = dc6_frame->header.width;
        spr_frame->height   = dc6_frame->header.height;
        spr_frame->offset_x = dc6_frame->header.offset_x;
        spr_frame->offset_y = dc6_frame->header.offset_y;

        uint32_t *pixels = malloc((size_t)spr_frame->width * spr_frame->height * 4);
        FAIL_IF_NULL(pixels);
        memset(pixels, 0, (size_t)spr_frame->width * spr_frame->height * 4);

        for (uint32_t idx = 0; idx < (spr_frame->width * spr_frame->height); idx++) {
            pixels[idx] = Palette->entries[dc6_frame->indexed_pixel_data[idx]];
        }

        SDL_UpdateTexture(spr_frame->texture, NULL, pixels, (int)spr_frame->width * 4);
        free(pixels);
    }

    dc6_free(DC6);
}

void sprite_free(struct Sprite *Sprite) {
    for (uint32_t i = 0; i < Sprite->frame_count; i++) {
        SDL_DestroyTexture(Sprite->frames[i].texture);
    }
    free(Sprite->frames);
    free(Sprite);
}

void sprite_draw(const struct Sprite *Sprite, const uint8_t frame_index, const int x, const int y) {
    const struct SpriteFrame *frame = &Sprite->frames[frame_index];

    const SDL_Rect dest = {x + frame->offset_x, y - (int)frame->height + frame->offset_y, (int)frame->width,
                           (int)frame->height};

    SDL_RenderCopy(sdl_renderer, frame->texture, NULL, &dest);
}

void sprite_draw_animated(struct Sprite *Sprite, const int x, const int y) {
    if (Sprite->ticks_per_frame == 0) {
        LOG_FATAL("Attempted to animate a Sprite with no ticks per frame!");
    }

    const uint64_t cur_ticks  = SDL_GetTicks64();
    uint64_t       tick_delta = cur_ticks - Sprite->last_ticks;

    while (tick_delta >= Sprite->ticks_per_frame) {
        Sprite->last_ticks += Sprite->ticks_per_frame;
        tick_delta         -= Sprite->ticks_per_frame;
        if (++Sprite->animation_index >= Sprite->frame_count) {
            Sprite->animation_index = 0;
        }
    }

    sprite_draw(Sprite, (uint8_t)Sprite->animation_index, x, y);
}

void sprite_draw_multi(const struct Sprite *Sprite, const uint8_t frame_index, const int x, const int y,
                       const int frames_x, const int frames_y) {
    int cur_x     = x;
    int cur_y     = y;
    int cur_frame = frame_index;

    for (int py = 0; py < frames_y; py++) {
        for (int px = 0; px < frames_x; px++) {
            const struct SpriteFrame *frame = &Sprite->frames[cur_frame++];
            SDL_Rect                  dest  = {cur_x, cur_y, (int)frame->width, (int)frame->height};
            SDL_RenderCopy(sdl_renderer, frame->texture, NULL, &dest);

            // sprite_draw(sprite, cur_frame++, cur_x, cur_y);
            cur_x += (int)Sprite->frames[cur_frame - 1].width;
        }
        cur_x  = x;
        cur_y += (int)Sprite->frames[cur_frame - 1].height;
    }
}

void sprite_set_blend_mode(const struct Sprite *Sprite, const SDL_BlendMode blend_mode) {
    for (uint32_t frame_idx = 0; frame_idx < Sprite->frame_count; frame_idx++) {
        SDL_SetTextureBlendMode(Sprite->frames[frame_idx].texture, blend_mode);
    }
}

void sprite_set_play_length(struct Sprite *Sprite, const double play_length) {
    Sprite->play_length     = play_length;
    Sprite->ticks_per_frame = (uint32_t)((1000 * play_length) / (double)Sprite->frame_count);
}
