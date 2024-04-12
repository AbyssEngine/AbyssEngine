#include "Scene.h"
#include "../common/Logging.h"
#include <stdlib.h>

void         *current_scene_ptr;
struct Scene *current_scene;
struct Scene *next_scene;

void scene_initialize(void) {
    current_scene     = NULL;
    next_scene        = NULL;
    current_scene_ptr = NULL;
}

void scene_finalize(void) {
    if (current_scene != NULL) {
        current_scene->free(current_scene_ptr);
    }

    current_scene_ptr = NULL;
    current_scene     = NULL;
    next_scene        = NULL;
}

void scene_render(void) {
    if (current_scene == NULL) {
        return;
    }

    current_scene->render(current_scene_ptr);
}

void scene_update(uint64_t delta) {
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

void scene_set(struct Scene *scene) {
    if (next_scene != NULL) {
        LOG_FATAL("Attempted to set a new Scene when one was already queued!");
    }

    next_scene = scene;
}
