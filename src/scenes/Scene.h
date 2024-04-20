#ifndef ABYSS_SCENE_H
#define ABYSS_SCENE_H

#include <stddef.h>
#include <stdint.h>

typedef struct Scene {
    void *(*create)(void);
    void (*render)(void *scene_ref);
    void (*update)(void *scene_ref, uint64_t delta);
    void (*free)(void **scene_ref);
} Scene;

void Scene_InitializeManager(void);
void Scene_DestroyManager(void);
void Scene_RenderCurrentScene(void);
void Scene_UpdateCurrentScene(uint64_t delta);
void Scene_Set(Scene *scene);

#define DESCRIBE_SCENE_CALLBACKS(C) extern Scene C
#define DEFINE_SCENE_CALLBACKS(C)   Scene C = {C##_Create, C##_Render, C##_Update, C##_Free}
#define SCENE_REF(C)                &C

#endif // ABYSS_SCENE_H
