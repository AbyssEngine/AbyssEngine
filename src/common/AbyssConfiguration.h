#ifndef ABYSS_CONFIG_H
#define ABYSS_CONFIG_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct AbyssConfiguration {
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

void abyss_configuration_load(const char *file_path);
void abyss_configuration_set_sane_defaults(void);
void abyss_configuration_free(void);
void abyss_configuration_add_mpq(const char *mpq_file);
void abyss_configuration_set_value(char *category, char *key, char *value);

extern struct AbyssConfiguration abyss_configuration;

#endif // ABYSS_CONFIG_H
