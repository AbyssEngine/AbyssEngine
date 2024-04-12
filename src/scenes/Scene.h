#ifndef ABYSS_SCENE_H
#define ABYSS_SCENE_H

#include <stddef.h>
#include <stdint.h>

struct Scene {
    void *(*create)(void);
    void (*render)(void *scene_ref);
    void (*update)(void *scene_ref, uint64_t delta);
    void (*free)(void *scene_ref);
};

void scene_initialize(void);
void scene_finalize(void);
void scene_render(void);
void scene_update(uint64_t delta);
void scene_set(struct Scene *scene);

#endif // ABYSS_SCENE_H
