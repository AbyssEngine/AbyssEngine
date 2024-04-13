#include "Label.h"
#include "../common/AbyssConfiguration.h"
#include "../common/Globals.h"
#include "../common/Logging.h"

struct Label *label_create(const char *font_path, const char *palette_name) {
    char dc6_path[4096];

    struct Label *result = malloc(sizeof(struct Label));
    FAIL_IF_NULL(result);
    memset(result, 0, sizeof(struct Label));

    memset(dc6_path, 0, sizeof(dc6_path));
    snprintf(dc6_path, sizeof(dc6_path), font_path, abyss_configuration.locale);
    strncat(dc6_path, ".DC6", sizeof(dc6_path) - strlen(dc6_path) - 1);

    result->Font    = font_load(font_path);
    result->DC6     = dc6_load(dc6_path);
    result->Palette = palette_get(palette_name);

    return result;
}

void label_free(struct Label *Label) {
    if (Label->texture != NULL) {
        SDL_DestroyTexture(Label->texture);
    }

    if (Label->text != NULL) {
        free(Label->text);
    }

    font_free(Label->Font);
    dc6_free(Label->DC6);

    free(Label);
}

void label_initialize_caches(void) { LOG_DEBUG("Initializing Label caches..."); }

void label_finalize_caches(void) { LOG_DEBUG("Finalizing Label caches..."); }

void label_set_text(struct Label *Label, const char *text) {
    if (Label->texture != NULL) {
        SDL_DestroyTexture(Label->texture);
    }

    if (Label->text != NULL) {
        if (strcmp(Label->text, text) == 0) {
            return;
        }

        free(Label->text);
    }

    if (text == NULL || text[0] == '\0') {
        Label->width  = 0;
        Label->height = 0;

        if (Label->texture != NULL) {
            SDL_DestroyTexture(Label->texture);
            Label->texture = NULL;
        }

        return;
    }

    Label->text   = strdup(text);
    Label->width  = 0;
    Label->height = 0;

    for (const char *ch = text; *ch; ch++) {
        struct FontGlyph *glyph  = font_get_glyph(Label->Font, *ch);
        struct DC6Frame  *frame  = &Label->DC6->frames[glyph->frame_index];
        Label->width            += glyph->width;
        Label->height            = Label->height > frame->header.height ? Label->height : frame->header.height;
    }

    if (Label->width == 0 || Label->height == 0) {
        return;
    }

    Label->texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, Label->width,
                                       Label->height);
    FAIL_IF_NULL(Label->texture);

    uint32_t *pixels = malloc((Label->width * Label->height) * sizeof(uint32_t));
    FAIL_IF_NULL(pixels);
    memset(pixels, 0, (Label->width * Label->height) * sizeof(uint32_t));

    int offset_x = 0;
    for (const char *ch = text; *ch; ch++) {
        struct FontGlyph *glyph = font_get_glyph(Label->Font, *ch);
        struct DC6Frame  *frame = &Label->DC6->frames[glyph->frame_index];

        for (uint32_t y = 0; y < frame->header.height; y++) {
            if (y >= Label->height) {
                break;
            }
            for (uint32_t x = 0; x < frame->header.width; x++) {
                if (x + offset_x >= Label->width) {
                    break;
                }
                const uint8_t  palette_index = frame->indexed_pixel_data[(y * frame->header.width) + x];
                const uint32_t pixel_index   = (y * Label->width) + x + offset_x;

                if (palette_index == 0) {
                    continue;
                }

                pixels[pixel_index] = Label->Palette->entries[palette_index];
            }
        }

        offset_x += glyph->width;
    }

    SDL_UpdateTexture(Label->texture, NULL, pixels, Label->width * 4);
    SDL_SetTextureBlendMode(Label->texture, SDL_BLENDMODE_BLEND);
    free(pixels);

    label_update_offsets(Label);
}

void label_draw(const struct Label *Label, const int x, const int y) {
    if (Label->texture == NULL) {
        return;
    }

    const SDL_Rect dest = {x + Label->offset_x, y + Label->offset_y, Label->width, Label->height};
    SDL_RenderCopy(sdl_renderer, Label->texture, NULL, &dest);
}

void label_set_color(struct Label *Label, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_SetTextureColorMod(Label->texture, r, g, b);
    SDL_SetTextureAlphaMod(Label->texture, a);
}

void label_set_align(struct Label *Label, label_align_t horizontal, label_align_t vertical) {
    Label->horizontal_align = horizontal;
    Label->vertical_align   = vertical;
    label_update_offsets(Label);
}
void label_update_offsets(struct Label *Label) {
    switch (Label->horizontal_align) {
    case LABEL_ALIGN_BEGIN:
        Label->offset_x = 0;
        break;
    case LABEL_ALIGN_CENTER:
        Label->offset_x = -Label->width / 2;
        break;
    case LABEL_ALIGN_END:
        Label->offset_x = -Label->width;
        break;
    default:
        LOG_FATAL("Invalid horizontal alignment value: %d", Label->horizontal_align);
        break;
    }

    switch (Label->vertical_align) {
    case LABEL_ALIGN_BEGIN:
        Label->offset_y = 0;
        break;
    case LABEL_ALIGN_CENTER:
        Label->offset_y = -Label->height / 2;
        break;
    case LABEL_ALIGN_END:
        Label->offset_y = -Label->height;
        break;
    default:
        LOG_FATAL("Invalid vertical alignment value: %d", Label->vertical_align);
        break;
    }
}