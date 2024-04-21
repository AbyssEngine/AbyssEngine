#include "AbyssConfiguration.h"
#include "Logging.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

typedef struct AbyssConfiguration {
    char  *base_path;
    char  *locale;
    char **mpqs;
    int    num_mpqs;
    bool   skip_intro_movies;
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
} AbyssConfiguration;

void AbyssConfiguration__SetSaneDefaults(void);
void AbyssConfiguration_SetValue(const char *category, const char *key, const char *value);

struct AbyssConfiguration abyss_configuration;
bool                      added_mpq = false;

static const char *default_mpqs[] = {"d2exp.MPQ",   "d2xmusic.MPQ", "d2xtalk.MPQ", "d2xvideo.MPQ",
                                     "d2data.MPQ",  "d2char.MPQ",   "d2music.MPQ", "d2sfx.MPQ",
                                     "d2video.MPQ", "d2speech.MPQ", NULL};

#define MAX_LINE_LEN       4096
#define IS_STR_EQUAL(X, Y) (strcmp(X, Y) == 0)

#define SET_PARAM_STR(X, Y)                                                                                            \
    if ((X) != NULL) {                                                                                                 \
        free(X);                                                                                                       \
    }                                                                                                                  \
    (X) = malloc(sizeof(char) * (strlen(Y) + 1));                                                                      \
    if ((X) == NULL) {                                                                                                 \
        LOG_FATAL("Failed to allocate memory.");                                                                       \
    }                                                                                                                  \
    memset(X, 0, sizeof(char) * (strlen(Y) + 1));                                                                      \
    strcat(X, Y)

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

void AbyssConfiguration_SetValue(const char *category, const char *key, const char *value) {
    if (IS_STR_EQUAL(category, "general")) {
        if (IS_STR_EQUAL(key, "basepath")) {
            if (strlen(abyss_configuration.base_path) != 0) {
                memset(abyss_configuration.base_path, 0, sizeof(char) * strlen(abyss_configuration.base_path));
            }
            SET_PARAM_STR(abyss_configuration.base_path, value);
            LOG_DEBUG("Setting base path to '%s'", abyss_configuration.base_path);
        } else if (IS_STR_EQUAL(key, "locale")) {
            if (strlen(abyss_configuration.locale) != 0) {
                memset(abyss_configuration.locale, 0, sizeof(char) * strlen(abyss_configuration.locale));
            }
            SET_PARAM_STR(abyss_configuration.locale, value);
            LOG_DEBUG("Setting locale to '%s'", abyss_configuration.locale);
        } else if (IS_STR_EQUAL(key, "skipintromovies")) {
            if (IS_STR_EQUAL(value, "true")) {
                abyss_configuration.skip_intro_movies = true;
            } else if (IS_STR_EQUAL(value, "false")) {
                abyss_configuration.skip_intro_movies = false;
            } else {
                LOG_FATAL("Invalid value '%s' for key '%s' in the configuration file!", value, key);
            }
            LOG_DEBUG("Setting fullscreen to '%s'", abyss_configuration.graphics.fullscreen ? "true" : "false");
        } else {
            LOG_FATAL("Invalid key '%s' in the configuration file!", key);
        }
    } else if (IS_STR_EQUAL(category, "graphics")) {
        if (IS_STR_EQUAL(key, "scalequality")) {
            if (strlen(abyss_configuration.graphics.scale_quality) != 0) {
                memset(abyss_configuration.graphics.scale_quality, 0,
                       sizeof(char) * strlen(abyss_configuration.graphics.scale_quality));
            }
            SET_PARAM_STR(abyss_configuration.graphics.scale_quality, value);
            LOG_DEBUG("Setting scale quality to '%s'", abyss_configuration.graphics.scale_quality);
        } else if (IS_STR_EQUAL(key, "initialscale")) {
            abyss_configuration.graphics.initial_scale = strtof(value, NULL);
            if (abyss_configuration.graphics.initial_scale <= 0.0f) {
                LOG_FATAL("Invalid value '%s' for key '%s' in the configuration file!", value, key);
            }
            LOG_DEBUG("Setting initial scale to '%f'", abyss_configuration.graphics.initial_scale);
        } else if (IS_STR_EQUAL(key, "fullscreen")) {
            if (IS_STR_EQUAL(value, "true")) {
                abyss_configuration.graphics.fullscreen = true;
            } else if (IS_STR_EQUAL(value, "false")) {
                abyss_configuration.graphics.fullscreen = false;
            } else {
                LOG_FATAL("Invalid value '%s' for key '%s' in the configuration file!", value, key);
            }
            LOG_DEBUG("Setting fullscreen to '%s'", abyss_configuration.graphics.fullscreen ? "true" : "false");
        } else {
            LOG_FATAL("Invalid key '%s' in the configuration file!", key);
        }
    } else if (IS_STR_EQUAL(category, "audio")) {
        if (IS_STR_EQUAL(key, "mastervolume")) {
            abyss_configuration.audio.master_volume = strtof(value, NULL);
            if (abyss_configuration.audio.master_volume < 0.0f || abyss_configuration.audio.master_volume > 1.0f) {
                LOG_FATAL("Invalid value '%s' for key '%s' in the configuration file!", value, key);
            }
            LOG_DEBUG("Setting master volume to '%f'", abyss_configuration.audio.master_volume);
        } else if (IS_STR_EQUAL(key, "musicvolume")) {
            abyss_configuration.audio.music_volume = strtof(value, NULL);
            if (abyss_configuration.audio.music_volume < 0.0f || abyss_configuration.audio.music_volume > 1.0f) {
                LOG_FATAL("Invalid value '%s' for key '%s' in the configuration file!", value, key);
            }
            LOG_DEBUG("Setting music volume to '%f'", abyss_configuration.audio.music_volume);
        } else if (IS_STR_EQUAL(key, "sfxvolume")) {
            abyss_configuration.audio.sfx_volume = strtof(value, NULL);
            if (abyss_configuration.audio.sfx_volume < 0.0f || abyss_configuration.audio.sfx_volume > 1.0f) {
                LOG_FATAL("Invalid value '%s' for key '%s' in the configuration file!", value, key);
            }
            LOG_DEBUG("Setting sfx volume to '%f'", abyss_configuration.audio.sfx_volume);
        } else if (IS_STR_EQUAL(key, "uivolume")) {
            abyss_configuration.audio.ui_volume = strtof(value, NULL);
            if (abyss_configuration.audio.ui_volume < 0.0f || abyss_configuration.audio.ui_volume > 1.0f) {
                LOG_FATAL("Invalid value '%s' for key '%s' in the configuration file!", value, key);
            }
            LOG_DEBUG("Setting ui volume to '%f'", abyss_configuration.audio.ui_volume);
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
            for (int i = 0; i < abyss_configuration.num_mpqs; i++) {
                free(abyss_configuration.mpqs[i]);
            }
            free(abyss_configuration.mpqs);
            abyss_configuration.num_mpqs = 0;
            abyss_configuration.mpqs     = calloc(0, sizeof(char *));
        }

        AbyssConfiguration_AddMpq(value);
    } else if (strlen(category) == 0) {
        LOG_FATAL("Invalid key '%s' outside of a category in the configuration file!", key);
    } else {
        LOG_FATAL("Invalid category '%s' in the configuration file!", category);
    }
}

void AbyssConfiguration_LoadSingleton(const char *file_path) {
    memset(&abyss_configuration, 0, sizeof(struct AbyssConfiguration));
    abyss_configuration.mpqs     = calloc(0, sizeof(char *));
    abyss_configuration.num_mpqs = 0;

    AbyssConfiguration__SetSaneDefaults();

    char category[MAX_LINE_LEN];
    char key[MAX_LINE_LEN];
    char value[MAX_LINE_LEN];

    memset(category, 0, sizeof(char) * MAX_LINE_LEN);
    memset(key, 0, sizeof(char) * MAX_LINE_LEN);
    memset(value, 0, sizeof(char) * MAX_LINE_LEN);

    FILE *ini_file = fopen(file_path, "r");
    if (ini_file == NULL) {
        LOG_FATAL("Could not find file '%s'!", file_path);
    }

    while (true) {
        char line[MAX_LINE_LEN];
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

        AbyssConfiguration_SetValue(category, key, value);
    }

    fclose(ini_file);
}

void AbyssConfiguration_DestroySingleton(void) {
    free(abyss_configuration.base_path);
    free(abyss_configuration.locale);
    free(abyss_configuration.graphics.scale_quality);

    for (int i = 0; i < abyss_configuration.num_mpqs; i++) {
        free(abyss_configuration.mpqs[i]);
    }

    free(abyss_configuration.mpqs);
}

void AbyssConfiguration_AddMpq(const char *mpq_file) {
    abyss_configuration.num_mpqs++;

    abyss_configuration.mpqs = realloc(abyss_configuration.mpqs, abyss_configuration.num_mpqs * sizeof(char *));
    FAIL_IF_NULL(abyss_configuration.mpqs);

    abyss_configuration.mpqs[abyss_configuration.num_mpqs - 1] = malloc(sizeof(char) * MAX_LINE_LEN);
    FAIL_IF_NULL(abyss_configuration.mpqs[abyss_configuration.num_mpqs - 1]);

    memset(abyss_configuration.mpqs[abyss_configuration.num_mpqs - 1], 0, sizeof(char) * MAX_LINE_LEN);
    strcat(abyss_configuration.mpqs[abyss_configuration.num_mpqs - 1], abyss_configuration.base_path);
    strcat(abyss_configuration.mpqs[abyss_configuration.num_mpqs - 1], mpq_file);
}
const char *AbyssConfiguration_GetLocale(void) { return abyss_configuration.locale; }

size_t AbyssConfiguration_GetMpqCount(void) { return abyss_configuration.num_mpqs; }

const char *AbyssConfiguration_GetMpqFileName(size_t index) { return abyss_configuration.mpqs[index]; }

float AbyssConfiguration_GetInitialScale(void) { return abyss_configuration.graphics.initial_scale; }

const char *AbyssConfiguration_GetScaleQuality(void) { return abyss_configuration.graphics.scale_quality; }

bool AbyssConfiguration_GetFullScreen(void) { return abyss_configuration.graphics.fullscreen; }

void AbyssConfiguration_SetFullScreen(const bool fullscreen) { abyss_configuration.graphics.fullscreen = fullscreen; }

float AbyssConfiguration_GetMasterVolume(void) { return abyss_configuration.audio.master_volume; }

float AbyssConfiguration_GetMusicVolume(void) { return abyss_configuration.audio.music_volume; }

float AbyssConfiguration_GetSfxVolume(void) { return abyss_configuration.audio.sfx_volume; }

float AbyssConfiguration_GetUiVolume(void) { return abyss_configuration.audio.ui_volume; }

bool AbyssConfiguration_GetSkipIntroMovies(void) { return abyss_configuration.skip_intro_movies; }

void AbyssConfiguration__SetSaneDefaults(void) {
    char base_path[4096];
    memset(base_path, 0, sizeof(char) * 4096);
    getcwd(base_path, 4096);
    SET_PARAM_STR(abyss_configuration.base_path, base_path);

    abyss_configuration.skip_intro_movies = false;

    added_mpq = false;
    for (const char **mpq_path = default_mpqs; *mpq_path != NULL; mpq_path++) {
        AbyssConfiguration_AddMpq(*mpq_path);
    }

    SET_PARAM_STR(abyss_configuration.graphics.scale_quality, "nearest");
    abyss_configuration.graphics.initial_scale = 1.0f;
    abyss_configuration.graphics.fullscreen    = false;

    abyss_configuration.audio.master_volume = 0.8f;
    abyss_configuration.audio.music_volume  = 1.0f;
    abyss_configuration.audio.sfx_volume    = 1.0f;
    abyss_configuration.audio.ui_volume     = 1.0f;

    SET_PARAM_STR(abyss_configuration.locale, "latin");
}
