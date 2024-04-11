#ifndef ABYSS_CONFIG_H
#define ABYSS_CONFIG_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct config {
    char  *base_path;
    char  *locale;
    char **mpqs;
    int    num_mpqs;
    struct {
        char *scale_quality;
        float initial_scale;
        bool  fullscreen;
    } graphics;
    struct {
        float master_volume;
        float music_volume;
        float sfx_volume;
        float ui_volume;
    } audio;
};

void config_load(const char *file_path);
void config_set_sane_defaults(void);
void config_free(void);
void config_add_mpq(const char *mpq_file);
void config_set(char *category, char *key, char *value);

extern struct config config;

#endif // ABYSS_CONFIG_H
