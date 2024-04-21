#include "MPQBlock.h"
#include "../util/Crypto.h"
#include <stdlib.h>
#include <string.h>

void MPQBlock_CalculateEncryptionSeed(struct MPQBlock *block, const char *file_name) {
    const char    *name    = strrchr(file_name, '\\') + 1;
    const uint32_t seed    = crypto_hash_string(name, 3);
    block->encryption_seed = (seed + block->file_position) ^ block->size_uncompressed;
}
