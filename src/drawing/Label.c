#include "Label.h"
#include "../common/AbyssConfiguration.h"
#include "../common/Globals.h"
#include "../common/Logging.h"

struct Label {
    DC6           *DC6;
    Font          *Font;
    const Palette *Palette;
    SDL_Texture   *texture;
    uint16_t       width;
    uint16_t       height;
    label_align_t  horizontal_align;
    label_align_t  vertical_align;
    int            offset_x;
    int            offset_y;
    char          *text;
};

void Label__UpdateOffsets(Label *Label);

Label *Label_Create(const char *font_path, const char *palette_name) {
    char dc6_path[4096];

    Label *result = malloc(sizeof(Label));
    FAIL_IF_NULL(result);
    memset(result, 0, sizeof(Label));

    memset(dc6_path, 0, sizeof(dc6_path));
    snprintf(dc6_path, sizeof(dc6_path), font_path, AbyssConfiguration_GetLocale());
    strncat(dc6_path, ".DC6", sizeof(dc6_path) - strlen(dc6_path) - 1);

    result->Font    = Font_Load(font_path);
    result->DC6     = DC6_Load(dc6_path);
    result->Palette = Palette_Get(palette_name);

    return result;
}

void Label_Destroy(Label **label) {
    if ((*label)->texture != NULL) {
        SDL_DestroyTexture((*label)->texture);
    }

    if ((*label)->text != NULL) {
        free((*label)->text);
    }

    Font_Destroy((*label)->Font);
    DC6_Destroy(&(*label)->DC6);

    free((*label));
    *label = NULL;
}

void Label_InitializeCaches(void) { LOG_DEBUG("Initializing Label caches..."); }

void Label_FinalizeCaches(void) { LOG_DEBUG("Finalizing Label caches..."); }

void Label_SetText(Label *label, const char *text) {
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
        uint16_t frame_index;
        uint8_t  frame_width;
        uint32_t frame_height;

        Font_GetGlyphMetrics(label->Font, *ch, &frame_index, &frame_width, NULL);
        DC6_GetFrameSize(label->DC6, frame_index, NULL, &frame_height);

        label->width  += frame_width;
        label->height  = label->height > frame_height ? label->height : frame_height;
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
        uint16_t frame_index;
        uint32_t frame_width;
        uint8_t  glyph_width;
        uint32_t frame_height;

        Font_GetGlyphMetrics(label->Font, *ch, &frame_index, &glyph_width, NULL);
        DC6_GetFrameSize(label->DC6, frame_index, &frame_width, &frame_height);

        const uint8_t *frame_pixel_data = DC6_GetFramePixelData(label->DC6, frame_index);

        for (uint32_t y = 0; y < frame_height; y++) {
            if (y >= label->height) {
                break;
            }
            for (uint32_t x = 0; x < frame_width; x++) {
                if (x + offset_x >= label->width) {
                    break;
                }
                const uint8_t  palette_index = frame_pixel_data[(y * frame_width) + x];
                const uint32_t pixel_index   = (y * label->width) + x + offset_x;

                if (palette_index == 0) {
                    continue;
                }

                pixels[pixel_index] = label->Palette->entries[palette_index];
            }
        }

        offset_x += glyph_width;
    }

    SDL_UpdateTexture(label->texture, NULL, pixels, label->width * 4);
    SDL_SetTextureBlendMode(label->texture, SDL_BLENDMODE_BLEND);
    free(pixels);

    Label__UpdateOffsets(label);
}

void Label_Draw(const Label *Label, int x, int y) {
    if (Label->texture == NULL) {
        return;
    }

    const SDL_Rect dest = {x + Label->offset_x, y + Label->offset_y, Label->width, Label->height};
    SDL_RenderCopy(sdl_renderer, Label->texture, NULL, &dest);
}

void Label_SetColor(Label *label, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_SetTextureColorMod(label->texture, r, g, b);
    SDL_SetTextureAlphaMod(label->texture, a);
}

void Label_SetAlignment(Label *label, label_align_t horizontal, label_align_t vertical) {
    label->horizontal_align = horizontal;
    label->vertical_align   = vertical;
    Label__UpdateOffsets(label);
}
void Label__UpdateOffsets(Label *label) {
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
    }
}
