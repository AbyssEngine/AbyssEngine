#include "config.h"
#include "log.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

config_t *config    = NULL;
bool      added_mpq = false;

static const char *default_mpqs[] = {"d2exp.mpq",  "d2xmusic.mpq", "d2xtalk.mpq", "d2xvideo.mpq", "d2data.mpq",
                                     "d2char.mpq", "d2music.mpq",  "d2sfx.mpq",   "d2video.mpq",  "d2speech.mpq"};

#define MAX_LINE_LEN       4096
#define IS_STR_EQUAL(X, Y) (strcmp(X, Y) == 0)

#define SET_PARAM_STR(X, Y)                                                                                            \
    if ((X) != NULL) {                                                                                                 \
        free(X);                                                                                                       \
    }                                                                                                                  \
    (X) = malloc(sizeof(char) * (strlen(Y) + 1));                                                                      \
    if (X == NULL) {                                                                                                   \
        LOG_FATAL("Failed to allocate memory.");                                                                       \
    }                                                                                                                  \
    memset(X, 0, sizeof(char) * (strlen(Y) + 1));                                                                      \
    strcat(X, Y);

static const char *valid_categories[] = {"general", "mpqs", "graphics", "audio", NULL};

char *trim_str(char *str) {
    size_t       len   = strlen(str);
    const size_t index = strcspn(str, "\r\n");

    if (index < len) {
        str[index] = '\0';
    }

    char *result = str;
    while (isspace(*result) || *result == '\t') {
        result++;
    }

    len       = strlen(result);
    char *end = result + len;

    while (end >= result && (isspace(*end) || *end == '\t')) {
        end--;
    }
    *(end + 1) = '\0';

    return result;
}

bool is_category(const char *str) {
    const size_t str_len = strlen(str);
    return (str_len >= 3) && (str[0] == '[') && (str[str_len - 1] == ']');
}

bool is_valid_category(const char *category) {
    for (int idx = 0; valid_categories[idx] != NULL; idx++) {
        if (strcmp(valid_categories[idx], category) != 0) {
            continue;
        }
        return true;
    }
    return false;
}

void extract_category(const char *str, char *out) {
    strcat(out, str + 1);
    out[strlen(str) - 2] = '\0';

    for (char *ch = out; *ch != '\0'; ch++) {
        *ch = (char)tolower(*ch);
    }
}

void extract_key_value(const char *str, char *key, char *value) {
    char *equal_ptr = strchr(str, '=');

    if (equal_ptr == NULL) {
        strcat(value, str);
        return;
    }

    *equal_ptr = '\0';
    strcat(key, str);

    const char *val_start = str + strlen(key) + 1;
    while (isspace(*val_start) || *val_start == '\t') {
        val_start++;
    }
    strcat(value, val_start);

    for (char *ch = key; *ch != '\0'; ch++) {
        if (isspace(*ch) || *ch == '\t') {
            *ch = '\0';
            break;
        }
        *ch = (char)tolower(*ch);
    }

    const size_t len = strlen(value);
    char        *end = value + len;
    while (end >= value && (isspace(*end) || *end == '\t')) {
        end--;
    }
    *(end + 1) = '\0';
}

void config_set(char *category, char *key, char *value) {
    if (IS_STR_EQUAL(category, "general")) {
        if (IS_STR_EQUAL(key, "basepath")) {
            if (strlen(config->base_path) != 0) {
                memset(config->base_path, 0, sizeof(char) * strlen(config->base_path));
            }
            SET_PARAM_STR(config->base_path, value);
            LOG_DEBUG("Setting base path to '%s'", config->base_path);
        } else if (IS_STR_EQUAL(key, "locale")) {
            if (strlen(config->locale) != 0) {
                memset(config->locale, 0, sizeof(char) * strlen(config->locale));
            }
            SET_PARAM_STR(config->locale, value);
            LOG_DEBUG("Setting locale to '%s'", config->locale);
        } else {
            LOG_FATAL("Invalid key '%s' in the configuration file!", key);
        }
    } else if (IS_STR_EQUAL(category, "graphics")) {
        if (IS_STR_EQUAL(key, "scalequality")) {
            if (strlen(config->graphics.scale_quality) != 0) {
                memset(config->graphics.scale_quality, 0, sizeof(char) * strlen(config->graphics.scale_quality));
            }
            SET_PARAM_STR(config->graphics.scale_quality, value);
            LOG_DEBUG("Setting scale quality to '%s'", config->graphics.scale_quality);
        } else if (IS_STR_EQUAL(key, "initialscale")) {
            config->graphics.initial_scale = strtof(value, NULL);
            if (config->graphics.initial_scale <= 0.0f) {
                LOG_FATAL("Invalid value '%s' for key '%s' in the configuration file!", value, key);
            }
            LOG_DEBUG("Setting initial scale to '%f'", config->graphics.initial_scale);
        } else if (IS_STR_EQUAL(key, "fullscreen")) {
            if (IS_STR_EQUAL(value, "true")) {
                config->graphics.fullscreen = true;
            } else if (IS_STR_EQUAL(value, "false")) {
                config->graphics.fullscreen = false;
            } else {
                LOG_FATAL("Invalid value '%s' for key '%s' in the configuration file!", value, key);
            }
            LOG_DEBUG("Setting fullscreen to '%s'", config->graphics.fullscreen ? "true" : "false");
        } else {
            LOG_FATAL("Invalid key '%s' in the configuration file!", key);
        }
    } else if (IS_STR_EQUAL(category, "audio")) {
        if (IS_STR_EQUAL(key, "mastervolume")) {
            config->audio.master_volume = strtof(value, NULL);
            if (config->audio.master_volume < 0.0f || config->audio.master_volume > 1.0f) {
                LOG_FATAL("Invalid value '%s' for key '%s' in the configuration file!", value, key);
            }
            LOG_DEBUG("Setting master volume to '%f'", config->audio.master_volume);
        } else if (IS_STR_EQUAL(key, "musicvolume")) {
            config->audio.music_volume = strtof(value, NULL);
            if (config->audio.music_volume < 0.0f || config->audio.music_volume > 1.0f) {
                LOG_FATAL("Invalid value '%s' for key '%s' in the configuration file!", value, key);
            }
            LOG_DEBUG("Setting music volume to '%f'", config->audio.music_volume);
        } else if (IS_STR_EQUAL(key, "sfxvolume")) {
            config->audio.sfx_volume = strtof(value, NULL);
            if (config->audio.sfx_volume < 0.0f || config->audio.sfx_volume > 1.0f) {
                LOG_FATAL("Invalid value '%s' for key '%s' in the configuration file!", value, key);
            }
            LOG_DEBUG("Setting sfx volume to '%f'", config->audio.sfx_volume);
        } else if (IS_STR_EQUAL(key, "uivolume")) {
            config->audio.ui_volume = strtof(value, NULL);
            if (config->audio.ui_volume < 0.0f || config->audio.ui_volume > 1.0f) {
                LOG_FATAL("Invalid value '%s' for key '%s' in the configuration file!", value, key);
            }
            LOG_DEBUG("Setting ui volume to '%f'", config->audio.ui_volume);
        } else {
            LOG_FATAL("Invalid key '%s' in the configuration file!", key);
        }
    } else if (IS_STR_EQUAL(category, "mpqs")) {
        if (strlen(key) != 0) {
            LOG_FATAL("Key/Value pair for '%s' not allowed in the MPQ "
                      "category in the configuration file!",
                      key);
        }

        if (!added_mpq) {
            added_mpq = true;
            for (int i = 0; i < config->num_mpqs; i++) {
                free(config->mpqs[i]);
            }
            free(config->mpqs);
            config->num_mpqs = 0;
            config->mpqs     = calloc(0, sizeof(char *));
        }

        config_add_mpq(value);
    } else if (strlen(category) == 0) {
        LOG_FATAL("Invalid key '%s' outside of a category in the configuration file!", key);
    } else {
        LOG_FATAL("Invalid category '%s' in the configuration file!", category);
    }
}

void config_load(const char *file_path) {
    config = malloc(sizeof(config_t));
    FAIL_IF_NULL(config);

    memset(config, 0, sizeof(config_t));
    config->mpqs     = calloc(0, sizeof(char *));
    config->num_mpqs = 0;

    config_set_sane_defaults();

    char *category = malloc(sizeof(char) * MAX_LINE_LEN);
    char *key      = malloc(sizeof(char) * MAX_LINE_LEN);
    char *value    = malloc(sizeof(char) * MAX_LINE_LEN);
    char *line     = malloc(sizeof(char) * MAX_LINE_LEN);

    FAIL_IF_NULL(category);
    FAIL_IF_NULL(key);
    FAIL_IF_NULL(value);
    FAIL_IF_NULL(line);

    memset(category, 0, sizeof(char) * MAX_LINE_LEN);
    memset(key, 0, sizeof(char) * MAX_LINE_LEN);
    memset(value, 0, sizeof(char) * MAX_LINE_LEN);

    FILE *ini_file = fopen(file_path, "r");
    if (ini_file == NULL) {
        LOG_FATAL("Could not find file '%s'!", file_path);
    }

    while (true) {
        memset(line, 0, sizeof(char) * MAX_LINE_LEN);
        if (fgets(line, MAX_LINE_LEN, ini_file) == NULL) {
            break;
        }

        char *comment_ptr = strchr(line, '#');
        if (comment_ptr != NULL) {
            *comment_ptr = '\0';
        }

        const char *trimmed_line = trim_str(line);

        if (strlen(trimmed_line) == 0) {
            continue;
        }

        if (is_category(trimmed_line)) {
            memset(category, 0, sizeof(char) * MAX_LINE_LEN);
            extract_category(trimmed_line, category);
            if (!is_valid_category(category)) {
                LOG_FATAL("Invalid category '%s' in '%s'!", category, file_path);
            }
            continue;
        }

        memset(key, 0, sizeof(char) * MAX_LINE_LEN);
        memset(value, 0, sizeof(char) * MAX_LINE_LEN);
        extract_key_value(trimmed_line, key, value);

        if ((strlen(key) == 0) && (strlen(value) == 0)) {
            continue;
        }

        config_set(category, key, value);
    }

    fclose(ini_file);
    free(line);
    free(value);
    free(key);
    free(category);
}

void config_free() {
    free(config->base_path);
    free(config->locale);
    free(config->graphics.scale_quality);

    for (int i = 0; i < config->num_mpqs; i++) {
        free(config->mpqs[i]);
    }

    free(config->mpqs);
    free(config);
}

void config_add_mpq(const char *mpq_file) {
    config->num_mpqs++;

    config->mpqs = realloc(config->mpqs, config->num_mpqs * sizeof(char *));
    FAIL_IF_NULL(config->mpqs);

    config->mpqs[config->num_mpqs - 1] = malloc(sizeof(char) * MAX_LINE_LEN);
    FAIL_IF_NULL(config->mpqs[config->num_mpqs - 1]);

    memset(config->mpqs[config->num_mpqs - 1], 0, sizeof(char) * MAX_LINE_LEN);
    strcat(config->mpqs[config->num_mpqs - 1], config->base_path);
    strcat(config->mpqs[config->num_mpqs - 1], mpq_file);
}

void config_set_sane_defaults() {
    char *base_path = malloc(sizeof(char) * 4096);
    FAIL_IF_NULL(base_path);

    memset(base_path, 0, sizeof(char) * 4096);
    getcwd(base_path, 4096);
    SET_PARAM_STR(config->base_path, base_path);
    free(base_path);

    added_mpq = false;
    for (int i = 0; i < 10; i++) {
        config_add_mpq(default_mpqs[i]);
    }

    SET_PARAM_STR(config->graphics.scale_quality, "nearest");
    config->graphics.initial_scale = 1.0f;
    config->graphics.fullscreen    = false;

    config->audio.master_volume = 0.8f;
    config->audio.music_volume  = 1.0f;
    config->audio.sfx_volume    = 1.0f;
    config->audio.ui_volume     = 1.0f;

    SET_PARAM_STR(config->locale, "latin");
}
