#ifndef ABYSS_WAV_DECOMPRESS
#define ABYSS_WAV_DECOMPRESS

#include <stdint.h>

uint8_t *wav_decompress(uint8_t *data, uint32_t buffer_len, uint8_t channel_count, uint32_t *result_size);

#endif // ABYSS_WAV_DECOMPRESS
