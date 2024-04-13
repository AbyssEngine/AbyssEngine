#ifndef ABYSS_BIT_READER_H
#define ABYSS_BIT_READER_H

#include <stdint.h>

struct BitReader {
    uint8_t *data_buffer;
    uint32_t buffer_len;
    uint8_t  buffer;
    uint8_t  bit_position;
};

struct BitReader *bit_reader_init(uint8_t *data_buffer, uint32_t buffer_len);
void              bit_reader_free(struct BitReader *br);
int               bit_reader_read_bit(struct BitReader *br);
int               bit_reader_read_bits(struct BitReader *br, int number_of_bits);

#endif // ABYSS_BIT_READER_H
