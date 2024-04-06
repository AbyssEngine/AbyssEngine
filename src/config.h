#ifndef ABYSS_CONFIG_H
#define ABYSS_CONFIG_H

#include <stdlib.h>
#include <string.h>

typedef struct config_s {
    char*   base_path;
    char**  mpqs;
    int     num_mpqs;
} config_t;

config_t*   config_load     (const char* file_path);
void        config_free     (config_t* config);
void        config_add_mpq  (config_t* config, const char* mpq_file);
void        config_set      (config_t* config, char* category, char* key, char* value);

#endif // ABYSS_CONFIG_H
