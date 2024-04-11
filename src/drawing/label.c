#include "label.h"
#include "../common/config.h"
#include "../common/globals.h"
#include "../common/log.h"

struct label *label_create(const char *font_path, const char *palette_name) {
    char dc6_path[4096];

    struct label *result = malloc(sizeof(struct label));
    FAIL_IF_NULL(result);
    memset(result, 0, sizeof(struct label));

    memset(dc6_path, 0, sizeof(dc6_path));
    snprintf(dc6_path, sizeof(dc6_path), font_path, config.locale);
    strncat(dc6_path, ".dc6", sizeof(dc6_path) - strlen(dc6_path) - 1);

    result->font    = font_load(font_path);
    result->dc6     = dc6_load(dc6_path);
    result->palette = palette_get(palette_name);

    return result;
}

void label_free(struct label *label) {
    if (label->texture != NULL) {
        SDL_DestroyTexture(label->texture);
    }

    if (label->text != NULL) {
        free(label->text);
    }

    font_free(label->font);
    dc6_free(label->dc6);

    free(label);
}

void label_initialize_caches(void) { LOG_DEBUG("Initializing label caches..."); }

void label_finalize_caches(void) { LOG_DEBUG("Finalizing label caches..."); }

void label_set_text(struct label *label, const char *text) {
    if (label->texture != NULL) {
        SDL_DestroyTexture(label->texture);
    }

    if (label->text != NULL) {
        if (strcmp(label->text, text) == 0) {
            return;
        }

        free(label->text);
    }

    if (text == NULL || text[0] == '\0') {
        label->width  = 0;
        label->height = 0;

        if (label->texture != NULL) {
            SDL_DestroyTexture(label->texture);
            label->texture = NULL;
        }
        
        return;
    }

    label->text   = strdup(text);
    label->width  = 0;
    label->height = 0;

    for (const char *ch = text; *ch; ch++) {
        struct font_glyph *glyph  = font_get_glyph(label->font, *ch);
        struct dc6_frame  *frame  = &label->dc6->frames[glyph->frame_index];
        label->width             += glyph->width;
        label->height             = label->height > frame->header.height ? label->height : frame->header.height;
    }

    if (label->width == 0 || label->height == 0) {
        return;
    }

    label->texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, label->width,
                                       label->height);
    FAIL_IF_NULL(label->texture);

    uint32_t *pixels = malloc((label->width * label->height) * sizeof(uint32_t));
    FAIL_IF_NULL(pixels);
    memset(pixels, 0, (label->width * label->height) * sizeof(uint32_t));

    int offset_x = 0;
    for (const char *ch = text; *ch; ch++) {
        struct font_glyph *glyph = font_get_glyph(label->font, *ch);
        struct dc6_frame  *frame = &label->dc6->frames[glyph->frame_index];

        for (uint32_t y = 0; y < frame->header.height; y++) {
            if (y >= label->height) {
                break;
            }
            for (uint32_t x = 0; x < frame->header.width; x++) {
                if (x + offset_x >= label->width) {
                    break;
                }
                const uint8_t  palette_index = frame->indexed_pixel_data[(y * frame->header.width) + x];
                const uint32_t pixel_index   = (y * label->width) + x + offset_x;

                if (palette_index == 0) {
                    continue;
                }

                pixels[pixel_index] = label->palette->entries[palette_index];
            }
        }

        offset_x += glyph->width;
    }

    SDL_UpdateTexture(label->texture, NULL, pixels, label->width * 4);
    SDL_SetTextureBlendMode(label->texture, SDL_BLENDMODE_BLEND);
    free(pixels);

    label_update_offsets(label);
}

void label_draw(const struct label *label, const int x, const int y) {
    if (label->texture == NULL) {
        return;
    }

    const SDL_Rect dest = {x + label->offset_x, y + label->offset_y, label->width, label->height};
    SDL_RenderCopy(sdl_renderer, label->texture, NULL, &dest);
}

void label_set_color(struct label *label, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_SetTextureColorMod(label->texture, r, g, b);
    SDL_SetTextureAlphaMod(label->texture, a);
}

void label_set_align(struct label *label, label_align_t horizontal, label_align_t vertical) {
    label->horizontal_align = horizontal;
    label->vertical_align   = vertical;
    label_update_offsets(label);
}
void label_update_offsets(struct label *label) {
    switch (label->horizontal_align) {
    case LABEL_ALIGN_BEGIN:
        label->offset_x = 0;
        break;
    case LABEL_ALIGN_CENTER:
        label->offset_x = -label->width / 2;
        break;
    case LABEL_ALIGN_END:
        label->offset_x = -label->width;
        break;
    default:
        LOG_FATAL("Invalid horizontal alignment value: %d", label->horizontal_align);
        break;
    }

    switch (label->vertical_align) {
    case LABEL_ALIGN_BEGIN:
        label->offset_y = 0;
        break;
    case LABEL_ALIGN_CENTER:
        label->offset_y = -label->height / 2;
        break;
    case LABEL_ALIGN_END:
        label->offset_y = -label->height;
        break;
    default:
        LOG_FATAL("Invalid vertical alignment value: %d", label->vertical_align);
        break;
    }
}
