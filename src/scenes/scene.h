#ifndef ABYSS_SCENE_H
#define ABYSS_SCENE_H

#include <stddef.h>
#include <stdint.h>

typedef struct scene_s {
    void *(*create)();

    void (*render)(void *scene_ref);

    void (*update)(void *scene_ref, uint32_t delta);

    void (*free)(void *scene_ref);
} scene_t;

void scene_initialize();

void scene_finalize();

void scene_render();

void scene_update(uint32_t delta);

void scene_set(scene_t *scene);

#endif // ABYSS_SCENE_H
