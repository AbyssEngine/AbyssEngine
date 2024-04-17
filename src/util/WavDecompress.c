#include "WavDecompress.h"

#include "../common/Logging.h"
#include "../common/MemoryStream.h"
#include <stdlib.h>
#include <string.h>

const int lookup[] = {0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x0010, 0x0011, 0x0013, 0x0015,
                      0x0017, 0x0019, 0x001C, 0x001F, 0x0022, 0x0025, 0x0029, 0x002D, 0x0032, 0x0037, 0x003C, 0x0042,
                      0x0049, 0x0050, 0x0058, 0x0061, 0x006B, 0x0076, 0x0082, 0x008F, 0x009D, 0x00AD, 0x00BE, 0x00D1,
                      0x00E6, 0x00FD, 0x0117, 0x0133, 0x0151, 0x0173, 0x0198, 0x01C1, 0x01EE, 0x0220, 0x0256, 0x0292,
                      0x02D4, 0x031C, 0x036C, 0x03C3, 0x0424, 0x048E, 0x0502, 0x0583, 0x0610, 0x06AB, 0x0756, 0x0812,
                      0x08E0, 0x09C3, 0x0ABD, 0x0BD0, 0x0CFF, 0x0E4C, 0x0FBA, 0x114C, 0x1307, 0x14EE, 0x1706, 0x1954,
                      0x1BDC, 0x1EA5, 0x21B6, 0x2515, 0x28CA, 0x2CDF, 0x315B, 0x364B, 0x3BB9, 0x41B2, 0x4844, 0x4F7E,
                      0x5771, 0x602F, 0x69CE, 0x7462, 0x7FFF};

const int lookup2[] = {-1, 0, -1, 4, -1, 2, -1, 6, -1, 1, -1, 5, -1, 3, -1, 7,
                       -1, 1, -1, 5, -1, 3, -1, 7, -1, 2, -1, 4, -1, 6, -1, 8};

uint8_t *wav_decompress(uint8_t *data, uint32_t buffer_len, int channel_count, uint32_t *result_size) {
    int  array1[] = {0x2C, 0x2C};
    int *array2   = malloc(sizeof(int) * channel_count);
    FAIL_IF_NULL(array2);
    memset(array2, 0, sizeof(int) * channel_count);

    struct MemoryStream *memory_stream_in  = memory_stream_create_from_existing_buffer(data, buffer_len);
    struct MemoryStream *memory_stream_out = memory_stream_create(true);

    // Skip the first byte
    memory_stream_read_skip_bytes(memory_stream_in, 1);

    uint8_t shift = memory_stream_read_uint8(memory_stream_in);

    for (int i = 0; i < channel_count; i++) {
        int16_t temp = memory_stream_read_int16(memory_stream_in);
        array2[i]    = (int)temp;
        memory_stream_write_int16(memory_stream_out, temp);
    }

    int channel = channel_count - 1;

    while (memory_stream_get_read_remaining(memory_stream_in)) {
        uint8_t value = memory_stream_read_uint8(memory_stream_in);

        if (channel_count == 2) {
            channel = 1 - channel;
        }

        if ((value & 0x80) != 0) {
            switch (value & 0x7F) {
            case 0:
                if (array1[channel] != 0) {
                    array1[channel]--;
                }
                memory_stream_write_int16(memory_stream_out, (int16_t)array2[channel]);
                break;
            case 1:
                array1[channel] += 8;
                if (array1[channel] > 0x58) {
                    array1[channel] = 0x58;
                }

                if (channel_count == 2) {
                    channel = 1 - channel;
                }
            case 2:
            default:
                array1[channel] -= 8;
                if (array1[channel] < 0) {
                    array1[channel] = 0;
                }

                if (channel_count == 2) {
                    channel = 1 - channel;
                }
            }

            continue;
        }

        if ((unsigned long)array1[channel] >= sizeof(lookup)) {
            LOG_FATAL("Failure decoding WAV: Attempted lookup outside of range!");
        }
        int temp1 = lookup[array1[channel]];
        int temp2 = temp1 >> shift;

        if ((value & 1) != 0) {
            temp2 += temp1 >> 0;
        }
        if ((value & 2) != 0) {
            temp2 += temp1 >> 1;
        }
        if ((value & 4) != 0) {
            temp2 += temp1 >> 2;
        }
        if ((value & 8) != 0) {
            temp2 += temp1 >> 3;
        }
        if ((value & 0x10) != 0) {
            temp2 += temp1 >> 4;
        }
        if ((value & 0x20) != 0) {
            temp2 += temp1 >> 5;
        }

        int temp3 = array2[channel];
        if ((value & 0x40) != 0) {
            temp3 -= temp2;
            if (temp3 < -32768) {
                temp3 = -32768;
            }
        } else {
            temp3 += temp2;
            if (temp3 > 32767) {
                temp3 = 32767;
            }
        }

        array2[channel] = temp3;
        memory_stream_write_int16(memory_stream_out, (int16_t)temp3);
        array1[channel] += lookup2[value & 0x1F];

        if (array1[channel] < 0) {
            array1[channel] = 0;
        } else if (array1[channel] > 0x58) {
            array1[channel] = 0x58;
        }
    }

    free(array2);

    uint8_t *result_buffer = memory_stream_out->buffer;
    *result_size           = (uint32_t)memory_stream_out->write_position;

    memory_stream_free(memory_stream_in);
    memory_stream_free(memory_stream_out);
    return result_buffer;
}
