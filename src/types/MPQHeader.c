#include "MPQHeader.h"
#include "../common/Logging.h"
#include <stdlib.h>

static const char mpq_magic[4] = {'M', 'P', 'Q', 0x1A};

void MPQHeader_Read(FILE *file, const char *mpq_path, MPQHeader *header) {
    fseek(file, 0, SEEK_SET);
    fread(header, sizeof(MPQHeader), 1, file);

    for (int i = 0; i < 4; i++) {
        if (header->magic[i] != mpq_magic[i]) {
            LOG_FATAL("Failed to load '%s' due to invalid header.", mpq_path);
        }
    }
}
