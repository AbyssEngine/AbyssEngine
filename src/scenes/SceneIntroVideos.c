#include "SceneIntroVideos.h"

#include "../common/AbyssConfiguration.h"
#include "../common/ResourcePaths.h"
#include "../drawing/Cursor.h"
#include "../managers/VideoManager.h"
#include "SceneMainMenu.h"
#include <stdlib.h>
#include <string.h>

enum IntroVideoState { INTRO_VIDEO_STATE_STARTUP_1, INTRO_VIDEO_STATE_STARTUP_2, INTRO_VIDEO_STATE_TRANSITION_TO_GAME };

typedef struct SceneIntroVideos {
    enum IntroVideoState intro_video_state;
} SceneIntroVideos;

#ifndef ABYSS_VERSION_TEXT
#define ABYSS_VERSION_TEXT "local build"
#endif // ABYSS_VERSION_TEXT

DEFINE_SCENE_CALLBACKS(IntroVideos);

void *IntroVideos_Create(void) {
    SceneIntroVideos *result = malloc(sizeof(SceneIntroVideos));

    Cursor_SetVisible(false);

    result->intro_video_state = INTRO_VIDEO_STATE_STARTUP_1;

    memset(result, 0, sizeof(SceneIntroVideos));
    return result;
}

void IntroVideos_Render(void *scene_ref) {
    SceneIntroVideos *scene = (SceneIntroVideos *)scene_ref;
    //
}

void IntroVideos_Update(void *scene_ref, uint64_t delta) {
    SceneIntroVideos *scene = (SceneIntroVideos *)scene_ref;

    if (AbyssConfiguration_GetSkipIntroMovies()) {
        Scene_Set(SCENE_REF(MainMenu));
        return;
    }

    switch (scene->intro_video_state++) {
    case INTRO_VIDEO_STATE_STARTUP_1:
        VideoManager_PlayVideo(VIDEO_BLIZZARD_STARTUP_1);
        return;
    case INTRO_VIDEO_STATE_STARTUP_2:
        VideoManager_PlayVideo(VIDEO_BLIZZARD_STARTUP_2);
        return;
    default:
        Scene_Set(SCENE_REF(MainMenu));
        return;
    }
}

void IntroVideos_Free(void **scene_ref) {
    SceneIntroVideos *scene = (SceneIntroVideos *)*scene_ref;
    free(scene);

    *scene_ref = NULL;
}
