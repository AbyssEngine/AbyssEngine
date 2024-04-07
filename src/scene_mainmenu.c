#include "scene_mainmenu.h"
#include "globals.h"
#include <stdlib.h>
#include <string.h>

scene_t scene_mainmenu = {
    scene_mainmenu_create,
    scene_mainmenu_render,
    scene_mainmenu_update,
    scene_mainmenu_free
};

void* scene_mainmenu_create() {
    mainmenu_t* result = malloc(sizeof(mainmenu_t));
    memset(result, 0, sizeof(mainmenu_t));
    
    result->background_sprite = sprite_load(TRADEMARK_SCREEN, PALETTE_SKY);
    
    return result;
}

void scene_mainmenu_render(void* scene_ref) {
    mainmenu_t* mainmenu = (mainmenu_t*) scene_ref;
    
    sprite_draw_multi(mainmenu->background_sprite, 0, 0, 0, 4, 3);
}

void scene_mainmenu_update(void* scene_ref, uint32_t delta) {
}

void scene_mainmenu_free(void* scene_ref) {
    mainmenu_t* mainmenu = (mainmenu_t*) scene_ref;
    
    sprite_free(mainmenu->background_sprite);
    free(mainmenu);
}
