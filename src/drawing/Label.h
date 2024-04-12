#ifndef ABYSS_LABEL_H
#define ABYSS_LABEL_H

#include "../types/DC6.h"
#include "../types/Font.h"
#include "../types/Palette.h"
#include <SDL2/SDL.h>

typedef uint8_t label_align_t;
#define LABEL_ALIGN_BEGIN  (label_align_t)0
#define LABEL_ALIGN_CENTER (label_align_t)1
#define LABEL_ALIGN_END    (label_align_t)2

struct Label {
    struct DC6     *DC6;
    struct Font    *Font;
    struct Palette *Palette;
    SDL_Texture    *texture;
    uint16_t        width;
    uint16_t        height;
    label_align_t   horizontal_align;
    label_align_t   vertical_align;
    int             offset_x;
    int             offset_y;
    char           *text;
};

struct Label *label_create(const char *font_path, const char *palette_name);
void          label_free(struct Label *Label);
void          label_set_text(struct Label *Label, const char *text);
void          label_draw(const struct Label *Label, int x, int y);
void          label_set_color(struct Label *Label, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void          label_set_align(struct Label *Label, label_align_t horizontal, label_align_t vertical);
void          label_initialize_caches(void);
void          label_finalize_caches(void);

void label_update_offsets(struct Label *Label);

#endif // ABYSS_LABEL_H
