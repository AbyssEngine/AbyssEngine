#include "Button.h"

#include "../common/Logging.h"
#include "../common/ResourcePaths.h"
#include "Label.h"
#include "Sprite.h"

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <string.h>

typedef struct ButtonLayout {
    ButtonType    button_type;
    char         *resource_name;
    char         *palette_name;
    char         *font_path;
    SDL_Rect      clickable_rect;
    uint8_t       segments_x;
    uint8_t       segments_y;
    int           base_frame;
    int           disabled_frame;
    uint32_t      disabled_color;
    int           text_offset;
    uint32_t      fixed_width;
    uint32_t      fixed_height;
    uint32_t      label_color;
    bool          toggleable;
    bool          allow_frame_change;
    bool          has_image;
    ButtonToolTip tooltip;
    int           tooltip_offset_x;
    int           tooltip_offset_y;
} ButtonLayout;

#define BTN_COLOR_LLIGHT_GRAY_ALPHA_75 0x808080c3
#define BTN_COLOR_LLIGHT_GRAY_ALPHA_100

static ButtonLayout button_layouts[] = {
    {.button_type        = BUTTON_TYPE_WIDE,
     .segments_x         = 2,
     .segments_y         = 1,
     .disabled_frame     = -1,
     .disabled_color     = BTN_COLOR_LLIGHT_GRAY_ALPHA_75,
     .text_offset        = 1,
     .resource_name      = "/data/global/ui/FrontEnd/WideButtonBlank.dc6",
     .palette_name       = PALETTE_UNITS,
     .font_path          = FONTS_FONTEXOCET10,
     .allow_frame_change = true,
     .has_image          = true,
     .label_color        = 0x646464ff},
    {.button_type = 0x9999}
};

struct Button {
    const ButtonLayout *layout;
    SDL_Texture        *normal_surface;
    SDL_Texture        *pressed_surface;
    SDL_Texture        *toggled_surface;
    SDL_Texture        *pressed_toggled_surface;
    SDL_Texture        *disabled_surface;
    void (*on_click)(void);
    void *opaque;
    bool  enabled;
    bool  pressed;
    bool  toggled;
    int   x;
    int   y;
    int   width;
    int   height;
    bool  visible;
    // ButtonToolTip *tooltip;
};

const ButtonLayout *Button__GetLayout(const ButtonType button_type);

Button *Button_Create(const ButtonType button_type, const char *text) {
    Button *button = malloc(sizeof(Button));
    memset(button, 0, sizeof(Button));

    button->layout = Button__GetLayout(button_type);

    Label  *label  = Label_Create(button->layout->font_path, PALETTE_UNITS);
    Sprite *sprite = Sprite_Create(button->layout->resource_name, button->layout->palette_name);

    if (button->layout->fixed_width > 0) {
        button->width = button->layout->fixed_width;
    } else {
        for (int i = 0; i < button->layout->segments_x; i++) {
            int w;
            Sprite_GetFrameSize(sprite, i, &w, NULL);
            button->width += w;
        }
    }

    if (button->layout->fixed_height > 0) {
        button->height = button->layout->fixed_height;
    } else {
        for (int i = 0; i < button->layout->segments_y; i++) {
            int h;
            Sprite_GetFrameSize(sprite, i * button->layout->segments_y, NULL, &h);
            button->height += h;
        }
    }

    Sprite_Destroy(&sprite);
    Label_Destroy(&label);
    return button;
}

void Button_Destroy(Button **button) {
    free(*button);
    *button = NULL;
}

const ButtonLayout *Button__GetLayout(const ButtonType button_type) {
    for (const ButtonLayout *button_layout = button_layouts; button_layout->button_type != 0xFFFF; button_layout++) {
        if (button_layout->button_type != button_type) {
            continue;
        }

        return button_layout;
    }

    LOG_FATAL("Could not locate button type.");
}
