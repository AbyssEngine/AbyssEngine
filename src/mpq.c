#include "mpq.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>

mpq_t* mpq_load(const char* mpq_path) {
    LOG_DEBUG("Loading '%s'...", mpq_path);

    mpq_t* result = malloc(sizeof(mpq_t));
    memset(result, 0, sizeof(mpq_t));
    
    result->file = fopen(mpq_path, "rb");
    if (result->file == NULL) {
        LOG_FATAL("Failed to load '%s'!", mpq_path);
    }

    mpq_header_read(result->file, mpq_path, &result->header);   
    result->hashes = mpq_hash_read_table(result->file, mpq_path, &result->header);

    return result;
}

void mpq_free(mpq_t* mpq) {
    fclose(mpq->file);
    free(mpq);
}
