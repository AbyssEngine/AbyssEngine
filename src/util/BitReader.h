#ifndef ABYSS_BIT_READER_H
#define ABYSS_BIT_READER_H

#include <stddef.h>
#include <stdint.h>

typedef struct BitReader BitReader;

BitReader *BitReader_Create(uint8_t *data_buffer, size_t buffer_len);
void       BitReader_Destroy(BitReader *br);
uint32_t   BitReader_ReadBit(BitReader *br);
uint32_t   BitReader_ReadBits(BitReader *br, int number_of_bits);

#endif // ABYSS_BIT_READER_H
