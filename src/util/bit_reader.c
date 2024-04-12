#include "bit_reader.h"
#include "../common/log.h"
#include <stdlib.h>
#include <string.h>

struct bit_reader *bit_reader_init(uint8_t *data_buffer, uint32_t buffer_len) {
    struct bit_reader *bit_reader = malloc(sizeof(struct bit_reader));
    memset(bit_reader, 0, sizeof(struct bit_reader));
    FAIL_IF_NULL(bit_reader);

    bit_reader->data_buffer = data_buffer;
    bit_reader->buffer_len  = buffer_len;
    return bit_reader;
}

void bit_reader_free(struct bit_reader *br) { free(br); }

int bit_reader_read_bit(struct bit_reader *br) {
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

int bit_reader_read_bits(struct bit_reader *br, int number_of_bits) {
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
