#include "config.h"
#include "log.h"
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

config_t *config = NULL;

#define MAX_LINE_LEN 4096
#define IS_STR_EQUAL(X, Y) (strcmp(X, Y)==0)

#define SET_PARAM_STR(X, Y)                      \
    if ((X) != NULL) { free(X); }                 \
    (X) = malloc(sizeof(char)*(strlen(Y)+1));     \
    memset(X, 0, sizeof(char)*(strlen(Y)+1));   \
    strcat(X, Y);

static const char *valid_categories[] = {
        "general",
        "mpqs",
        NULL
};

char *trim_str(char *str) {
    size_t len = strlen(str);
    size_t index = strcspn(str, "\r\n");

    if (index < len) {
        str[index] = '\0';
    }

    char *result = str;
    while (isspace(*result) || *result == '\t') {
        result++;
    }

    len = strlen(result);
    char *end = result + len;

    while (end >= result && (isspace(*end) || *end == '\t')) {
        end--;
    }
    *(end + 1) = '\0';

    return result;
}

bool is_category(char *str) {
    size_t str_len = strlen(str);
    return (str_len >= 3) && (str[0] == '[') && (str[str_len - 1] == ']');
}

bool is_valid_category(char *category) {
    for (int idx = 0; valid_categories[idx] != NULL; idx++) {
        if (strcmp(valid_categories[idx], category) != 0) {
            continue;
        }
        return true;
    }
    return false;
}

void extract_category(char *str, char *out) {
    strcat(out, str + 1);
    out[strlen(str) - 2] = '\0';

    for (char *ch = out; *ch != '\0'; ch++) {
        *ch = (char) tolower((unsigned char) *ch);
    }
}

void extract_key_value(char *str, char *key, char *value) {
    char *equal_ptr = strchr(str, '=');

    if (equal_ptr == NULL) {
        strcat(value, str);
        return;
    }

    *equal_ptr = '\0';
    strcat(key, str);
    strcat(value, str + strlen(str) + 1);

    for (char *ch = key; *ch != '\0'; ch++) {
        if (isspace(*ch) || *ch == '\t') {
            *ch = '\0';
            break;
        }
        *ch = (char) tolower((unsigned char) *ch);
    }
}

void config_set(char *category, char *key, char *value) {

    if (IS_STR_EQUAL(category, "general")) {
        if (IS_STR_EQUAL(key, "basepath")) {
            SET_PARAM_STR(config->base_path, value);
            LOG_DEBUG("Setting base path to '%s'", config->base_path);
        } else if (IS_STR_EQUAL(key, "locale")) {
            SET_PARAM_STR(config->locale, value);
            LOG_DEBUG("Setting locale to '%s'", config->locale);
        } else {
            LOG_FATAL("Invalid key '%s' in the configuration file!", key);
        }
    } else if (IS_STR_EQUAL(category, "mpqs")) {
        if (strlen(key) != 0) {
            LOG_FATAL("Key/Value pair for '%s' not allowed in the MPQ "
                      "category in the configuration file!", key);
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
    memset(config, 0, sizeof(config_t));
    config->mpqs = calloc(0, sizeof(char *));

    char *category = malloc(sizeof(char) * MAX_LINE_LEN);
    char *key = malloc(sizeof(char) * MAX_LINE_LEN);
    char *value = malloc(sizeof(char) * MAX_LINE_LEN);
    char *line = malloc(sizeof(char) * MAX_LINE_LEN);

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
        char *trimmed_line = trim_str(line);

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

    for (int i = 0; i < config->num_mpqs; i++) {
        free(config->mpqs[i]);
    }

    free(config->mpqs);
    free(config);
}

void config_add_mpq(const char *mpq_file) {
    config->num_mpqs++;
    config->mpqs = realloc(config->mpqs, config->num_mpqs * sizeof(char *));
    config->mpqs[config->num_mpqs - 1] = malloc(sizeof(char) * MAX_LINE_LEN);
    memset(config->mpqs[config->num_mpqs - 1], 0, sizeof(char) * MAX_LINE_LEN);
    strcat(config->mpqs[config->num_mpqs - 1], config->base_path);
    strcat(config->mpqs[config->num_mpqs - 1], mpq_file);
}
