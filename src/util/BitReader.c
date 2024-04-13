#include "BitReader.h"
#include "../common/Logging.h"
#include <stdlib.h>
#include <string.h>

struct BitReader *bit_reader_init(uint8_t *data_buffer, uint32_t buffer_len) {
    struct BitReader *BitReader = malloc(sizeof(struct BitReader));
    memset(BitReader, 0, sizeof(struct BitReader));
    FAIL_IF_NULL(BitReader);

    BitReader->data_buffer = data_buffer;
    BitReader->buffer_len  = buffer_len;
    return BitReader;
}

void bit_reader_free(struct BitReader *br) { free(br); }

int bit_reader_read_bit(struct BitReader *br) {
    if (br->bit_position == 0) {
        if (br->buffer_len == 0) {
            return -1;
        }
        br->buffer_len--;
        br->buffer = *br->data_buffer;
        br->data_buffer++;
    }

    int bit          = (br->buffer >> br->bit_position) & 1;
    br->bit_position = (br->bit_position + 1) % 8;

    return bit;
}

int bit_reader_read_bits(struct BitReader *br, int number_of_bits) {
    int result = 0;
    for (int i = 0; i < number_of_bits; i++) {
        int bit = bit_reader_read_bit(br);
        if (bit < 0) {
            return -1;
        }
        result |= (bit << i);
    }
    return result;
}
