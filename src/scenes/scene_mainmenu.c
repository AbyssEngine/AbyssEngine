#include "scene_mainmenu.h"
#include "../audio/audio_stream.h"
#include "../common/globals.h"
#include "../common/log.h"
#include <stdlib.h>
#include <string.h>

#ifndef ABYSS_VERSION_TEXT
#define ABYSS_VERSION_TEXT "local build"
#endif // ABYSS_VERSION_TEXT

struct scene scene_mainmenu = {scene_mainmenu_create, scene_mainmenu_render, scene_mainmenu_update,
                               scene_mainmenu_free};

void *scene_mainmenu_create(void) {
    struct audio_stream *stream = audio_stream_create(MUSIC_TITLE);
    audio_stream_free(stream);

    struct mainmenu *result = malloc(sizeof(struct mainmenu));
    FAIL_IF_NULL(result);
    memset(result, 0, sizeof(struct mainmenu));

    result->background_sprite         = sprite_load(GAME_SELECT_SCREEN, PALETTE_SKY);
    result->d2logo_black_left_sprite  = sprite_load(D2LOGO_BLACK_LEFT, PALETTE_UNITS);
    result->d2logo_black_right_sprite = sprite_load(D2LOGO_BLACK_RIGHT, PALETTE_UNITS);
    result->d2logo_fire_left_sprite   = sprite_load(D2LOGO_FIRE_LEFT, PALETTE_UNITS);
    result->d2logo_fire_right_sprite  = sprite_load(D2LOGO_FIRE_RIGHT, PALETTE_UNITS);

    sprite_set_blend_mode(result->d2logo_black_left_sprite, SDL_BLENDMODE_BLEND);
    sprite_set_blend_mode(result->d2logo_black_right_sprite, SDL_BLENDMODE_BLEND);
    sprite_set_blend_mode(result->d2logo_fire_left_sprite, SDL_BLENDMODE_ADD);
    sprite_set_blend_mode(result->d2logo_fire_right_sprite, SDL_BLENDMODE_ADD);

    result->copyright_label = label_create(FONTS_FONTFORMAL10, PALETTE_SKY);
    label_set_text(
        result->copyright_label,
        "Abyss Engine is neither developed by, nor endorsed by Blizzard/Activision or its parent company Microsoft.");
    label_set_color(result->copyright_label, 0xFF, 0xFF, 0x8C, 0xFF);
    label_set_align(result->copyright_label, LABEL_ALIGN_CENTER, LABEL_ALIGN_CENTER);

    result->build_label = label_create(FONTS_FONT8, PALETTE_STATIC);
    label_set_text(result->build_label, ABYSS_VERSION_TEXT);
    label_set_color(result->build_label, 0xFF, 0xFF, 0xFF, 0xFF);
    label_set_align(result->build_label, LABEL_ALIGN_END, LABEL_ALIGN_BEGIN);
    return result;
}

void scene_mainmenu_render(void *scene_ref) {
    const struct mainmenu *mainmenu = (struct mainmenu *)scene_ref;

    sprite_draw_multi(mainmenu->background_sprite, 0, 0, 0, 4, 3);
    sprite_draw_animated(mainmenu->d2logo_black_left_sprite, 400, 120);
    sprite_draw_animated(mainmenu->d2logo_black_right_sprite, 400, 120);
    sprite_draw_animated(mainmenu->d2logo_fire_left_sprite, 400, 120);
    sprite_draw_animated(mainmenu->d2logo_fire_right_sprite, 400, 120);

    label_draw(mainmenu->copyright_label, 400, 590);
    label_draw(mainmenu->build_label, 797, 1);
}

void scene_mainmenu_update(__attribute__((unused)) void *scene_ref, __attribute__((unused)) uint64_t delta) {}

void scene_mainmenu_free(void *scene_ref) {
    struct mainmenu *mainmenu = (struct mainmenu *)scene_ref;

    label_free(mainmenu->copyright_label);
    sprite_free(mainmenu->d2logo_black_left_sprite);
    sprite_free(mainmenu->d2logo_black_right_sprite);
    sprite_free(mainmenu->d2logo_fire_left_sprite);
    sprite_free(mainmenu->d2logo_fire_right_sprite);
    sprite_free(mainmenu->background_sprite);

    free(mainmenu);
}
