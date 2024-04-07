#ifndef ABYSS_SCENE_MAINMENU_H
#define ABYSS_SCENE_MAINMENU_H

#include "scene.h"
#include "../drawing/sprite.h"

extern scene_t scene_mainmenu;

typedef struct mainmenu_s {
    sprite_t *background_sprite;
    sprite_t *d2logo_black_left_sprite;
    sprite_t *d2logo_black_right_sprite;
    sprite_t *d2logo_fire_left_sprite;
    sprite_t *d2logo_fire_right_sprite;
} mainmenu_t;

void *scene_mainmenu_create();

void scene_mainmenu_render(void *scene_ref);

void scene_mainmenu_update(void *scene_ref, uint32_t delta);

void scene_mainmenu_free(void *scene_ref);


#endif // ABYSS_SCENE_MAINMENU_H

