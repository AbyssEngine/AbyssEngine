#ifndef ABYSS_CRYPTO_H
#define ABYSS_CRYPTO_H

#include <stdint.h>
#include <stdio.h>

extern uint32_t crypto_buffer[0x500];

void      crypto_init(void);
uint32_t  crypto_hash_string(const char *key, uint32_t hash_type);
uint32_t *crypto_decrypt_table(FILE *file, uint32_t size, const char *name);
uint64_t  crypto_hash_file_name(const char *file_name);
void      crypto_decrypt(uint32_t *buffer, uint32_t size, uint32_t seed);
void      crypto_decrypt_bytes(char *buffer, uint32_t size, uint32_t seed);
#endif // ABYSS_CRYPTO_H
