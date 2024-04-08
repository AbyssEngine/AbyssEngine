#include "crypto.h"
#include "../common/log.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

uint32_t crypto_buffer[0x500];

void crypto_init() {
    LOG_DEBUG("Initializing cryptographic table.");
    uint32_t seed = 0x00100001;

    for (int index1 = 0; index1 < 0x100; index1++) {
        int index2 = index1;
        for (int i = 0; i < 5; i++) {
            seed                   = (seed * 125 + 3) % 0x2AAAAB;
            const uint32_t temp1         = (seed & 0xFFFF) << 0x10;
            seed                   = (seed * 125 + 3) % 0x2AAAAB;
            const uint32_t temp2         = (seed & 0xFFFF);
            crypto_buffer[index2]  = temp1 | temp2;
            index2                += 0x100;
        }
    }
}

uint32_t crypto_hash_string(const char *key, const uint32_t hash_type) {
    uint32_t seed1 = 0x7FED7FED;
    uint32_t seed2 = 0xEEEEEEEE;

    const size_t len = strlen(key);
    for (size_t i = 0; i < len; i++) {
        const char ch = toupper(key[i]);
        seed1   = crypto_buffer[(hash_type * 0x100) + (uint32_t)ch] ^ (seed1 + seed2);
        seed2   = (uint32_t)ch + seed1 + seed2 + (seed2 << 5) + 3;
    }

    return seed1;
}

uint32_t *crypto_decrypt_table(FILE *file, uint32_t size, const char *name) {
    size *= 4;

    uint32_t  seed  = crypto_hash_string(name, 3);
    uint32_t  seed2 = 0xEEEEEEEE;
    uint32_t *table = malloc(sizeof(uint32_t) * size);

    FAIL_IF_NULL(table);
    uint8_t   buff[4];

    for (uint32_t i = 0; i < size; i++) {
        seed2 += crypto_buffer[0x400 + (seed & 0xFF)];

        if (fread(&buff, sizeof(uint8_t), 4, file) != 4) {
            LOG_FATAL("Error decrypting table!");
        }
        uint32_t result  = buff[0] | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24);
        result          ^= seed + seed2;

        seed     = ((~seed << 21) + 0x11111111) | (seed >> 11);
        seed2    = result + seed2 + (seed2 << 5) + 3;
        table[i] = result;
    }

    return table;
}

uint64_t crypto_hash_file_name(const char *file_name) {
    const uint32_t a = crypto_hash_string(file_name, 1);
    const uint32_t b = crypto_hash_string(file_name, 2);
    return (((uint64_t)a) << 32) | (uint64_t)b;
}
