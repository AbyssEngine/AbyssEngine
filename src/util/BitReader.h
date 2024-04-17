#ifndef ABYSS_BIT_READER_H
#define ABYSS_BIT_READER_H

#include <stddef.h>
#include <stdint.h>

struct BitReader {
    uint8_t *data_buffer;
    size_t   buffer_len;
    int      offset;
    int      bits_read;
};

struct BitReader *bit_reader_init(uint8_t *data_buffer, size_t buffer_len);
void              bit_reader_free(struct BitReader *br);
uint32_t          bit_reader_read_bit(struct BitReader *br);
uint32_t          bit_reader_read_bits(struct BitReader *br, int number_of_bits);

#endif // ABYSS_BIT_READER_H
