#include "BitReader.h"
#include "../common/Logging.h"
#include <stdlib.h>
#include <string.h>

#define TWOS_COMPLIMENT_NEGATIVE_ONE 4294967295

struct BitReader {
    uint8_t *data_buffer;
    size_t   buffer_len;
    int      offset;
    int      bits_read;
};

struct BitReader *BitReader_Create(uint8_t *data_buffer, size_t buffer_len) {
    struct BitReader *bit_reader = malloc(sizeof(struct BitReader));
    memset(bit_reader, 0, sizeof(struct BitReader));
    FAIL_IF_NULL(bit_reader);

    bit_reader->data_buffer = data_buffer;
    bit_reader->buffer_len  = buffer_len;
    return bit_reader;
}

void BitReader_Destroy(BitReader *br) { free(br); }

uint32_t BitReader_ReadBit(BitReader *br) {
    uint32_t result = (uint32_t)(br->data_buffer[br->offset >> 3] >> (uint32_t)(br->offset & 7)) & 1;
    br->offset++;
    br->bits_read++;

    return result;
}

uint32_t BitReader_ReadBits(BitReader *br, int number_of_bits) {
    if (number_of_bits == 0) {
        return 0;
    }

    uint32_t result = 0;

    for (int i = 0; i < number_of_bits; i++) {
        result |= BitReader_ReadBit(br) << i;
    }

    return result;
}
