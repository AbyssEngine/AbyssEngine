#ifndef ABYSS_SCENE_MAINMENU_H
#define ABYSS_SCENE_MAINMENU_H

#include "../drawing/label.h"
#include "../drawing/sprite.h"
#include "scene.h"

extern struct scene scene_mainmenu;

struct mainmenu {
    struct sprite *background_sprite;
    struct sprite *d2logo_black_left_sprite;
    struct sprite *d2logo_black_right_sprite;
    struct sprite *d2logo_fire_left_sprite;
    struct sprite *d2logo_fire_right_sprite;
    struct label  *copyright_label;
    struct label  *build_label;
};

void *scene_mainmenu_create(void);
void  scene_mainmenu_render(void *scene_ref);
void  scene_mainmenu_update(__attribute__((unused)) void *scene_ref, __attribute__((unused)) uint64_t delta);
void  scene_mainmenu_free(void *scene_ref);

#endif // ABYSS_SCENE_MAINMENU_H
