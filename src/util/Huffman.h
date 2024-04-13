#ifndef ABYSS_HUFFMAN_H
#define ABYSS_HUFFMAN_H

#include "BitReader.h"

struct LinkedNode {
    int                decompressed_value;
    int                weight;
    struct LinkedNode *parent;
    struct LinkedNode *child_0;
    struct LinkedNode *prev;
    struct LinkedNode *next;
};

struct LinkedNode *huffman_create_linked_node(int decompressed_value, int weight);
struct LinkedNode *huffman_get_child1(struct LinkedNode *node);
struct LinkedNode *huffman_insert(struct LinkedNode *node, struct LinkedNode *other);
struct LinkedNode *huffman_decode(struct BitReader *BitReader, struct LinkedNode *head);
struct LinkedNode *huffman_build_list(uint16_t *prime_table);
struct LinkedNode *huffman_insert_node(struct LinkedNode *tail, int decomp);
void               huffman_adjust_tree(struct LinkedNode *new_node);
struct LinkedNode *huffman_build_tree(struct LinkedNode *tail);
void               huffman_free_linked_node(struct LinkedNode *node);
uint8_t           *huffman_decompress(uint8_t *buffer, uint32_t buffer_len, uint32_t *result_size);
#endif // ABYSS_HUFFMAN_H
