#include "../src/common/RingBuffer.h"
#include <assert.h>
#include <string.h>

int main(int argc, char **argv) {
    char buffer_in[27] = "abcdefghijklmnopqrstuvwxyz";
    char buffer_out[50];

    struct RingBuffer *rb = ring_buffer_create(8);
    assert(rb != NULL);

    ring_buffer_write(rb, buffer_in, 5);

    memset(buffer_out, 0, 50);
    ring_buffer_read(rb, buffer_out, 3);
    assert(strcmp(buffer_out, "abc") == 0);

    memset(buffer_out, 0, 50);
    ring_buffer_read(rb, buffer_out, 2);
    assert(strcmp(buffer_out, "de") == 0);

    memset(buffer_out, 0, 50);
    ring_buffer_write(rb, buffer_in, 7);
    ring_buffer_read(rb, buffer_out, 7);
    assert(strcmp(buffer_out, "abcdefg") == 0);

    memset(buffer_out, 0, 50);
    ring_buffer_write(rb, buffer_in, 7);
    ring_buffer_read(rb, buffer_out, 7);
    assert(strcmp(buffer_out, "abcdefg") == 0);

    ring_buffer_free(&rb);
    assert(rb == NULL);
    return 0;
}
