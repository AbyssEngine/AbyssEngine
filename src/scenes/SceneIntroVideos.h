#ifndef ABYSS_SCENE_INTRO_VIDEOS_H
#define ABYSS_SCENE_INTRO_VIDEOS_H

#include "Scene.h"

DESCRIBE_SCENE_CALLBACKS(IntroVideos);

void *IntroVideos_Create(void);
void  IntroVideos_Render(void *scene_ref);
void  IntroVideos_Update(void *scene_ref, uint64_t delta);
void  IntroVideos_Free(void **scene_ref);

#endif // ABYSS_SCENE_INTRO_VIDEOS_H
