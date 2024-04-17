#include "BitReader.h"
#include "../common/Logging.h"
#include <stdlib.h>
#include <string.h>

#define TWOS_COMPLIMENT_NEGATIVE_ONE 4294967295
#define BYTE_LENGTH                  8
#define ONE_BIT                      0x01
#define FOUR_BYTES                   (BYTE_LENGTH * 4)

struct BitReader *bit_reader_init(uint8_t *data_buffer, size_t buffer_len) {
    struct BitReader *bit_reader = malloc(sizeof(struct BitReader));
    memset(bit_reader, 0, sizeof(struct BitReader));
    FAIL_IF_NULL(bit_reader);

    bit_reader->data_buffer = data_buffer;
    bit_reader->buffer_len  = buffer_len;
    return bit_reader;
}

void bit_reader_free(struct BitReader *br) { free(br); }

uint32_t bit_reader_read_bit(struct BitReader *br) {
    uint32_t result =
        (uint32_t)(br->data_buffer[br->offset / BYTE_LENGTH] >> (uint32_t)(br->offset % BYTE_LENGTH)) & ONE_BIT;
    br->offset++;
    br->bits_read++;

    return result;
}

uint32_t bit_reader_read_bits(struct BitReader *br, int number_of_bits) {
    if (number_of_bits == 0) {
        return 0;
    }

    uint32_t result = 0;

    for (int i = 0; i < number_of_bits; i++) {
        result |= bit_reader_read_bit(br) << i;
    }

    return result;
}
