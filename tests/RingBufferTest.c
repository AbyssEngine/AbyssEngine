#include "../src/common/RingBuffer.h"
#include <assert.h>
#include <string.h>

int main(int argc, char **argv) {
    (void)(argc);
    (void)(argv);

    char buffer_in[27] = "abcdefghijklmnopqrstuvwxyz";
    char buffer_out[50];

    struct RingBuffer *rb = RingBuffer_Create(8);
    assert(rb != NULL);

    RingBuffer_Write(rb, buffer_in, 5);

    memset(buffer_out, 0, 50);
    RingBuffer_Read(rb, buffer_out, 3);
    assert(strcmp(buffer_out, "abc") == 0);

    memset(buffer_out, 0, 50);
    RingBuffer_Read(rb, buffer_out, 2);
    assert(strcmp(buffer_out, "de") == 0);

    memset(buffer_out, 0, 50);
    RingBuffer_Write(rb, buffer_in, 7);
    RingBuffer_Read(rb, buffer_out, 7);
    assert(strcmp(buffer_out, "abcdefg") == 0);

    memset(buffer_out, 0, 50);
    RingBuffer_Write(rb, buffer_in, 7);
    RingBuffer_Read(rb, buffer_out, 7);
    assert(strcmp(buffer_out, "abcdefg") == 0);

    RingBuffer_Free(&rb);
    assert(rb == NULL);
    return 0;
}
