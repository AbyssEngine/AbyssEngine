#include "Scene.h"
#include "../common/Logging.h"
#include <stdlib.h>

void  *current_scene_ptr;
Scene *current_scene;
Scene *next_scene;

void Scene_InitializeManager(void) {
    current_scene     = NULL;
    next_scene        = NULL;
    current_scene_ptr = NULL;
}

void Scene_DestroyManager(void) {
    if (current_scene != NULL) {
        current_scene->free(&current_scene_ptr);
    }

    current_scene_ptr = NULL;
    current_scene     = NULL;
    next_scene        = NULL;
}

void Scene_RenderCurrentScene(void) {
    if (current_scene == NULL) {
        return;
    }

    current_scene->render(current_scene_ptr);
}

void Scene_UpdateCurrentScene(uint64_t delta) {
    if (next_scene != NULL) {
        if (current_scene != NULL) {
            current_scene->free(&current_scene_ptr);
        }

        current_scene     = next_scene;
        next_scene        = NULL;
        current_scene_ptr = current_scene->create();
    }

    if (current_scene == NULL) {
        return;
    }

    current_scene->update(current_scene_ptr, delta);
}

void Scene_Set(Scene *scene) {
    if (next_scene != NULL) {
        LOG_FATAL("Attempted to set a new Scene when one was already queued!");
    }

    next_scene = scene;
}
