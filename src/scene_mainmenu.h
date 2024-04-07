#ifndef ABYSS_SCENE_MAINMENU_H
#define ABYSS_SCENE_MAINMENU_H

#include "scene.h"
#include "sprite.h"

extern scene_t scene_mainmenu;

typedef struct mainmenu_s {
    sprite_t* background_sprite;
} mainmenu_t;

void*   scene_mainmenu_create   ();
void    scene_mainmenu_render   (void* scene_ref);
void    scene_mainmenu_update   (void* scene_ref, uint32_t delta);
void    scene_mainmenu_free     (void* scene_ref);


#endif // ABYSS_SCENE_MAINMENU_H

