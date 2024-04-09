#ifndef ABYSS_LABEL_H
#define ABYSS_LABEL_H

#include "../types/dc6.h"
#include "../types/font.h"
#include "../types/palette.h"
#include <SDL2/SDL.h>

typedef uint8_t label_align_t;
#define LABEL_ALIGN_BEGIN  (label_align_t)0
#define LABEL_ALIGN_CENTER (label_align_t)1
#define LABEL_ALIGN_END    (label_align_t)2

typedef struct label_s {
    dc6_t        *dc6;
    font_t       *font;
    palette_t    *palette;
    SDL_Texture  *texture;
    uint16_t      width;
    uint16_t      height;
    label_align_t horizontal_align;
    label_align_t vertical_align;
    int           offset_x;
    int           offset_y;
} label_t;

label_t *label_create(const char *font_path, const char *palette_name);
void     label_free(label_t *label);
void     label_set_text(label_t *label, const char *text);
void     label_draw(const label_t *label, int x, int y);
void     label_set_color(label_t *label, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void     label_set_align(label_t *label, label_align_t horizontal, label_align_t vertical);
void     label_initialize_caches();
void     label_finalize_caches();

void label_update_offsets(label_t *label);

#endif // ABYSS_LABEL_H
