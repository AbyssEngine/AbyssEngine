#ifndef ABYSS_WAV_DECOMPRESS
#define ABYSS_WAV_DECOMPRESS

#include <stdint.h>

uint8_t *WAV_Decompress(uint8_t *data, uint32_t buffer_len, int channel_count, uint32_t *result_size);

#endif // ABYSS_WAV_DECOMPRESS
