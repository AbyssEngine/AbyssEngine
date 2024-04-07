#include "sprite.h"
#include "log.h"
#include "dc6.h"
#include "palette.h"
#include "globals.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* get_extension(const char* path) {
    char* ext = strrchr(path, '.')+1;
    if (strlen(path) <= (ext-path)) {
        LOG_FATAL("Invalid path extension for '%s'.", path); 
    }

    char* result = strdup(ext);
    for (char* ch=result; *ch; ch++) {
        *ch = tolower(*ch);
    }
    
    return result;
}

sprite_t* sprite_load(const char* path, const char* palette_name) {
    sprite_t* result = malloc(sizeof(sprite_t));
    memset(result, 0, sizeof(sprite_t));
    
    char* file_ext = get_extension(path);
    
    if (strcmp(file_ext, "dc6") == 0) {
        sprite_load_dc6(result, path, palette_name);
    } else {
        LOG_FATAL("Could not load sprite: '%s' is an unsupported file extension.", file_ext);
    }
    
    free(file_ext);
    return result;
}

void sprite_load_dc6(sprite_t* sprite, const char* path, const char* palette_name) {
    dc6_t* dc6 = dc6_load(path);
    palette_t* palette = palette_get(palette_name);
   
    sprite->frame_count     = dc6->header.directions * dc6->header.frames_per_direction;
    sprite->frames          = malloc(sizeof(sprite_frame_t)*sprite->frame_count);
    sprite->last_ticks      = SDL_GetTicks();
    sprite->animation_index = 0;
    
    sprite_set_play_length(sprite, 1.0);
    
    for (int i=0; i<sprite->frame_count; i++) {
        dc6_frame_t*    dc6_frame = &dc6->frames[i];
        sprite_frame_t* spr_frame = &sprite->frames[i];
       
        spr_frame->texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STATIC, dc6_frame->header.width, dc6_frame->header.height);
        
        spr_frame->width    = dc6_frame->header.width;
        spr_frame->height   = dc6_frame->header.height;
        spr_frame->offset_x = dc6_frame->header.offset_x;        
        spr_frame->offset_y = dc6_frame->header.offset_y;
        
        uint32_t* pixels = malloc(4 * spr_frame->width * spr_frame->height);
        memset(pixels, 0, spr_frame->width * spr_frame->height);
        
        for (uint32_t idx=0; idx<(spr_frame->width*spr_frame->height); idx++) {
            pixels[idx] = palette->entries[dc6_frame->indexed_pixel_data[idx]];
        }

        SDL_UpdateTexture(spr_frame->texture, NULL, (void*)pixels, spr_frame->width*4);
        free(pixels);
    }
    
    dc6_free(dc6);
}

void sprite_free(sprite_t* sprite) {
    for (int i=0; i<sprite->frame_count; i++) {
        SDL_DestroyTexture(sprite->frames[i].texture);
    }
    free(sprite->frames);
    free(sprite);
}

void sprite_draw(sprite_t* sprite, uint8_t frame_index, int x, int y) {
    sprite_frame_t* frame = &sprite->frames[frame_index];
    
    SDL_Rect dest = {
        x + frame->offset_x,
        y - frame->height + frame->offset_y,
        frame->width,
        frame->height
    };
    
    SDL_RenderCopy(sdl_renderer, frame->texture, NULL, &dest);
}

void sprite_draw_animated(sprite_t* sprite, int x, int y) {
    if (sprite->ticks_per_frame == 0) {
        LOG_FATAL("Attempted to animate a sprite with no ticks per frame!");
    }

    uint16_t cur_ticks = SDL_GetTicks();
    uint16_t tick_delta = cur_ticks - sprite->last_ticks;
    
    while (tick_delta >= sprite->ticks_per_frame) {
        sprite->last_ticks += sprite->ticks_per_frame;
        tick_delta -= sprite->ticks_per_frame;
        if (++sprite->animation_index >= sprite->frame_count) {
            sprite->animation_index = 0;
        }
    }
    
    sprite_draw(sprite, sprite->animation_index, x, y);
}

void sprite_draw_multi(sprite_t* sprite, uint8_t frame_index, int x, int y, int frames_x, int frames_y) {
    int cur_x       = x;
    int cur_y       = y;
    int cur_frame   = frame_index;
    
    for (int y=0; y<frames_y; y++) {
        for (int x=0; x<frames_x; x++) {
            sprite_frame_t* frame = &sprite->frames[cur_frame++];
            SDL_Rect dest = {cur_x, cur_y, frame->width, frame->height };
            SDL_RenderCopy(sdl_renderer, frame->texture, NULL, &dest);
        
            //sprite_draw(sprite, cur_frame++, cur_x, cur_y);
            cur_x += sprite->frames[cur_frame-1].width;
        }
        cur_x = x;
        cur_y += sprite->frames[cur_frame-1].height;
    }
}

void sprite_set_blend_mode(sprite_t* sprite, SDL_BlendMode blend_mode) {
    for (int frame_idx=0; frame_idx<sprite->frame_count; frame_idx++) {
        SDL_SetTextureBlendMode(sprite->frames[frame_idx].texture, blend_mode);
    }
}

void sprite_set_play_length(sprite_t* sprite, float play_length) {
    sprite->play_length     = play_length;
    sprite->ticks_per_frame = (1000 * play_length) / sprite->frame_count;
}

