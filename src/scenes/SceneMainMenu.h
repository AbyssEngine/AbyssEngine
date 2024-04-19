#ifndef ABYSS_SCENE_MAINMENU_H
#define ABYSS_SCENE_MAINMENU_H

#include "../drawing/Label.h"
#include "../drawing/Sprite.h"
#include "Scene.h"

DESCRIBE_SCENE_CALLBACKS(MainMenu);

void *MainMenu_Create(void);
void  MainMenu_Render(void *scene_ref);
void  MainMenu_Update(void *scene_ref, uint64_t delta);
void  MainMenu_Free(void **scene_ref);

#endif // ABYSS_SCENE_MAINMENU_H
