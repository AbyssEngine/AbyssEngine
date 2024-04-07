#include "scene.h"
#include "../common/log.h"

void    *current_scene_ptr;
scene_t *current_scene;
scene_t *next_scene;

void scene_initialize() {
    current_scene     = NULL;
    next_scene        = NULL;
    current_scene_ptr = NULL;
}

void scene_finalize() {
    if (current_scene != NULL) {
        current_scene->free(current_scene_ptr);
    }

    current_scene_ptr = NULL;
    current_scene     = NULL;
    next_scene        = NULL;
}

void scene_render() {
    if (current_scene == NULL) {
        return;
    }

    current_scene->render(current_scene_ptr);
}

void scene_update(uint32_t delta) {
    if (next_scene != NULL) {
        if (current_scene != NULL) {
            current_scene->free(current_scene_ptr);
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

void scene_set(scene_t *scene) {
    if (next_scene != NULL) {
        LOG_FATAL("Attempted to set a new scene when one was already queued!");
    }

    next_scene = scene;
}
