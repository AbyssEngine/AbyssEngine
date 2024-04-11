#include "sprite.h"
#include "../common/globals.h"
#include "../common/log.h"
#include "../types/dc6.h"
#include "../types/palette.h"
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
        *ch = tolower(*ch);
    }

    return result;
}

struct sprite *sprite_load(const char *path, const char *palette_name) {
    struct sprite *result = malloc(sizeof(struct sprite));
    FAIL_IF_NULL(result);

    memset(result, 0, sizeof(struct sprite));

    char *file_ext = get_extension(path);

    if (strcmp(file_ext, "dc6") == 0) {
        sprite_load_dc6(result, path, palette_name);
    } else {
        LOG_FATAL("Could not load sprite: '%s' is an unsupported file extension.", file_ext);
    }

    free(file_ext);
    return result;
}

void sprite_load_dc6(struct sprite *sprite, const char *path, const char *palette_name) {
    struct dc6      *dc6     = dc6_load(path);
    const palette_t *palette = palette_get(palette_name);

    sprite->frame_count     = dc6->header.directions * dc6->header.frames_per_direction;
    sprite->frames          = malloc(sizeof(struct sprite_frame) * sprite->frame_count);
    sprite->last_ticks      = SDL_GetTicks64();
    sprite->animation_index = 0;

    FAIL_IF_NULL(sprite->frames);

    sprite_set_play_length(sprite, 1.0);

    for (uint32_t i = 0; i < sprite->frame_count; i++) {
        const struct dc6_frame *dc6_frame = &dc6->frames[i];
        struct sprite_frame    *spr_frame = &sprite->frames[i];

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
            pixels[idx] = palette->entries[dc6_frame->indexed_pixel_data[idx]];
        }

        SDL_UpdateTexture(spr_frame->texture, NULL, pixels, (int)spr_frame->width * 4);
        free(pixels);
    }

    dc6_free(dc6);
}

void sprite_free(struct sprite *sprite) {
    for (uint32_t i = 0; i < sprite->frame_count; i++) {
        SDL_DestroyTexture(sprite->frames[i].texture);
    }
    free(sprite->frames);
    free(sprite);
}

void sprite_draw(const struct sprite *sprite, const uint8_t frame_index, const int x, const int y) {
    const struct sprite_frame *frame = &sprite->frames[frame_index];

    const SDL_Rect dest = {x + frame->offset_x, y - frame->height + frame->offset_y, (int)frame->width,
                           (int)frame->height};

    SDL_RenderCopy(sdl_renderer, frame->texture, NULL, &dest);
}

void sprite_draw_animated(struct sprite *sprite, const int x, const int y) {
    if (sprite->ticks_per_frame == 0) {
        LOG_FATAL("Attempted to animate a sprite with no ticks per frame!");
    }

    const uint64_t cur_ticks  = SDL_GetTicks64();
    uint64_t       tick_delta = cur_ticks - sprite->last_ticks;

    while (tick_delta >= sprite->ticks_per_frame) {
        sprite->last_ticks += sprite->ticks_per_frame;
        tick_delta         -= sprite->ticks_per_frame;
        if (++sprite->animation_index >= sprite->frame_count) {
            sprite->animation_index = 0;
        }
    }

    sprite_draw(sprite, (uint8_t)sprite->animation_index, x, y);
}

void sprite_draw_multi(const struct sprite *sprite, const uint8_t frame_index, const int x, const int y,
                       const int frames_x, const int frames_y) {
    int cur_x     = x;
    int cur_y     = y;
    int cur_frame = frame_index;

    for (int py = 0; py < frames_y; py++) {
        for (int px = 0; px < frames_x; px++) {
            const struct sprite_frame *frame = &sprite->frames[cur_frame++];
            SDL_Rect                   dest  = {cur_x, cur_y, (int)frame->width, (int)frame->height};
            SDL_RenderCopy(sdl_renderer, frame->texture, NULL, &dest);

            // sprite_draw(sprite, cur_frame++, cur_x, cur_y);
            cur_x += (int)sprite->frames[cur_frame - 1].width;
        }
        cur_x  = x;
        cur_y += (int)sprite->frames[cur_frame - 1].height;
    }
}

void sprite_set_blend_mode(const struct sprite *sprite, const SDL_BlendMode blend_mode) {
    for (uint32_t frame_idx = 0; frame_idx < sprite->frame_count; frame_idx++) {
        SDL_SetTextureBlendMode(sprite->frames[frame_idx].texture, blend_mode);
    }
}

void sprite_set_play_length(struct sprite *sprite, const float play_length) {
    sprite->play_length     = play_length;
    sprite->ticks_per_frame = (uint32_t)((1000 * play_length) / (float)sprite->frame_count);
}
