#include "SceneMainMenu.h"
#include "../common/Globals.h"
#include "../common/Logging.h"
#include "../drawing/Cursor.h"
#include "../managers/AudioManager.h"
#include <stdlib.h>
#include <string.h>

typedef struct SceneMainMenu {
    Sprite *background_sprite;
    Sprite *d2logo_black_left_sprite;
    Sprite *d2logo_black_right_sprite;
    Sprite *d2logo_fire_left_sprite;
    Sprite *d2logo_fire_right_sprite;
    Label  *disclaimer_label;
    Label  *copyright_label;
    Label  *build_label;
} SceneMainMenu;

DEFINE_SCENE_CALLBACKS(MainMenu);

void *MainMenu_Create(void) {
    Cursor_SetVisible(true);

    AudioManager_PlayMusic(MUSIC_TITLE, true);

    SceneMainMenu *result = malloc(sizeof(SceneMainMenu));
    FAIL_IF_NULL(result);
    memset(result, 0, sizeof(SceneMainMenu));

    result->background_sprite         = Sprite_Create(GAME_SELECT_SCREEN, PALETTE_SKY);
    result->d2logo_black_left_sprite  = Sprite_Create(D2LOGO_BLACK_LEFT, PALETTE_UNITS);
    result->d2logo_black_right_sprite = Sprite_Create(D2LOGO_BLACK_RIGHT, PALETTE_UNITS);
    result->d2logo_fire_left_sprite   = Sprite_Create(D2LOGO_FIRE_LEFT, PALETTE_UNITS);
    result->d2logo_fire_right_sprite  = Sprite_Create(D2LOGO_FIRE_RIGHT, PALETTE_UNITS);

    Sprite_SetBlendMode(result->d2logo_black_left_sprite, SDL_BLENDMODE_BLEND);
    Sprite_SetBlendMode(result->d2logo_black_right_sprite, SDL_BLENDMODE_BLEND);
    Sprite_SetBlendMode(result->d2logo_fire_left_sprite, SDL_BLENDMODE_ADD);
    Sprite_SetBlendMode(result->d2logo_fire_right_sprite, SDL_BLENDMODE_ADD);

    result->disclaimer_label = Label_Create(FONTS_FONTFORMAL10, PALETTE_SKY);
    Label_SetText(result->disclaimer_label, ABYSS_TEXT_MAIN_MENU_DISCLAIMER);
    Label_SetColor(result->disclaimer_label, 0xFF, 0xFF, 0x8C, 0xFF);
    Label_SetAlignment(result->disclaimer_label, LABEL_ALIGN_CENTER, LABEL_ALIGN_CENTER);

    result->copyright_label = Label_Create(FONTS_FONTFORMAL10, PALETTE_SKY);
    Label_SetText(result->copyright_label, ABYSS_COPYRIGHT);
    Label_SetColor(result->copyright_label, 0xFF, 0xFF, 0x8C, 0xFF);
    Label_SetAlignment(result->copyright_label, LABEL_ALIGN_CENTER, LABEL_ALIGN_CENTER);

    result->build_label = Label_Create(FONTS_FONT8, PALETTE_STATIC);
    Label_SetText(result->build_label, ABYSS_VERSION_TEXT);
    Label_SetColor(result->build_label, 0xFF, 0xFF, 0xFF, 0xFF);
    Label_SetAlignment(result->build_label, LABEL_ALIGN_END, LABEL_ALIGN_BEGIN);
    return result;
}

void MainMenu_Render(void *scene_ref) {
    const SceneMainMenu *mainmenu = (SceneMainMenu *)scene_ref;

    Sprite_DrawMulti(mainmenu->background_sprite, 0, 0, 0, 4, 3);
    Sprite_DrawAnimated(mainmenu->d2logo_black_left_sprite, 400, 120);
    Sprite_DrawAnimated(mainmenu->d2logo_black_right_sprite, 400, 120);
    Sprite_DrawAnimated(mainmenu->d2logo_fire_left_sprite, 400, 120);
    Sprite_DrawAnimated(mainmenu->d2logo_fire_right_sprite, 400, 120);

    Label_Draw(mainmenu->copyright_label, 400, 578);
    Label_Draw(mainmenu->disclaimer_label, 400, 590);
    Label_Draw(mainmenu->build_label, 797, 1);
}

void MainMenu_Update(void *scene_ref, uint64_t delta) {
    (void)(scene_ref);
    (void)(delta);
}

void MainMenu_Free(void **scene_ref) {
    SceneMainMenu *mainmenu = *((SceneMainMenu **)scene_ref);

    Label_Destroy(&mainmenu->copyright_label);
    Label_Destroy(&mainmenu->disclaimer_label);
    Sprite_Destroy(&mainmenu->d2logo_black_left_sprite);
    Sprite_Destroy(&mainmenu->d2logo_black_right_sprite);
    Sprite_Destroy(&mainmenu->d2logo_fire_left_sprite);
    Sprite_Destroy(&mainmenu->d2logo_fire_right_sprite);
    Sprite_Destroy(&mainmenu->background_sprite);

    free(*scene_ref);
    *scene_ref = NULL;
}
