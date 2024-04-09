#include "mpq_header.h"
#include "../common/log.h"
#include <stdlib.h>

static const char mpq_magic[4] = {'M', 'P', 'Q', 0x1A};

void mpq_header_read(FILE *file, const char *mpq_path, mpq_header_t *header) {
    fseek(file, 0, SEEK_SET);
    fread(header, sizeof(mpq_header_t), 1, file);

    for (int i = 0; i < 4; i++) {
        if (header->magic[i] != mpq_magic[i]) {
            LOG_FATAL("Failed to load '%s' due to invalid header.", mpq_path);
        }
    }
}
