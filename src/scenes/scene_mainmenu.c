#include "scene_mainmenu.h"
#include "../common/globals.h"
#include "../common/log.h"
#include <stdlib.h>
#include <string.h>

scene_t scene_mainmenu = {scene_mainmenu_create, scene_mainmenu_render, scene_mainmenu_update, scene_mainmenu_free};

void *scene_mainmenu_create() {
    mainmenu_t *result = malloc(sizeof(mainmenu_t));
    FAIL_IF_NULL(result);
    memset(result, 0, sizeof(mainmenu_t));

    result->background_sprite         = sprite_load(GAME_SELECT_SCREEN, PALETTE_SKY);
    result->d2logo_black_left_sprite  = sprite_load(D2LOGO_BLACK_LEFT, PALETTE_UNITS);
    result->d2logo_black_right_sprite = sprite_load(D2LOGO_BLACK_RIGHT, PALETTE_UNITS);
    result->d2logo_fire_left_sprite   = sprite_load(D2LOGO_FIRE_LEFT, PALETTE_UNITS);
    result->d2logo_fire_right_sprite  = sprite_load(D2LOGO_FIRE_RIGHT, PALETTE_UNITS);

    sprite_set_blend_mode(result->d2logo_black_left_sprite, SDL_BLENDMODE_BLEND);
    sprite_set_blend_mode(result->d2logo_black_right_sprite, SDL_BLENDMODE_BLEND);
    sprite_set_blend_mode(result->d2logo_fire_left_sprite, SDL_BLENDMODE_ADD);
    sprite_set_blend_mode(result->d2logo_fire_right_sprite, SDL_BLENDMODE_ADD);

    return result;
}

void scene_mainmenu_render(void *scene_ref) {
    const mainmenu_t *mainmenu = (mainmenu_t *)scene_ref;

    sprite_draw_multi(mainmenu->background_sprite, 0, 0, 0, 4, 3);
    sprite_draw_animated(mainmenu->d2logo_black_left_sprite, 400, 120);
    sprite_draw_animated(mainmenu->d2logo_black_right_sprite, 400, 120);
    sprite_draw_animated(mainmenu->d2logo_fire_left_sprite, 400, 120);
    sprite_draw_animated(mainmenu->d2logo_fire_right_sprite, 400, 120);
}

void scene_mainmenu_update(void *scene_ref, uint32_t delta) {}

void scene_mainmenu_free(void *scene_ref) {
    mainmenu_t *mainmenu = (mainmenu_t *)scene_ref;

    sprite_free(mainmenu->background_sprite);
    sprite_free(mainmenu->d2logo_black_left_sprite);
    sprite_free(mainmenu->d2logo_black_right_sprite);
    sprite_free(mainmenu->d2logo_fire_left_sprite);
    sprite_free(mainmenu->d2logo_fire_right_sprite);

    free(mainmenu);
}
