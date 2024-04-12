#ifndef ABYSS_SCENE_MAINMENU_H
#define ABYSS_SCENE_MAINMENU_H

#include "../drawing/Label.h"
#include "../drawing/Sprite.h"
#include "Scene.h"

extern struct Scene scene_main_menu;

struct SceneMainMenu {
    struct Sprite *background_sprite;
    struct Sprite *d2logo_black_left_sprite;
    struct Sprite *d2logo_black_right_sprite;
    struct Sprite *d2logo_fire_left_sprite;
    struct Sprite *d2logo_fire_right_sprite;
    struct Label  *copyright_label;
    struct Label  *build_label;
};

void *scene_mainmenu_create(void);
void  scene_mainmenu_render(void *scene_ref);
void  scene_mainmenu_update(__attribute__((unused)) void *scene_ref, __attribute__((unused)) uint64_t delta);
void  scene_mainmenu_free(void *scene_ref);

#endif // ABYSS_SCENE_MAINMENU_H
