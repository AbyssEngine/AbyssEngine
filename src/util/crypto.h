#ifndef ABYSS_CRYPTO_H
#define ABYSS_CRYPTO_H

#include <stdint.h>
#include <stdio.h>

extern uint32_t crypto_buffer[0x500];

void crypto_init();

uint32_t crypto_hash_string(const char *key, uint32_t hash_type);

uint32_t *crypto_decrypt_table(FILE *file, uint32_t size, const char *name);

uint64_t crypto_hash_file_name(const char *file_name);

#endif // ABYSS_CRYPTO_H
