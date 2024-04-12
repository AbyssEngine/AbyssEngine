#ifndef ABYSS_HUFFMAN_H
#define ABYSS_HUFFMAN_H

#include "bit_reader.h"

struct linked_node {
    int                 decompressed_value;
    int                 weight;
    struct linked_node *parent;
    struct linked_node *child_0;
    struct linked_node *prev;
    struct linked_node *next;
};

struct linked_node *huffman_create_linked_node(int decompressed_value, int weight);
struct linked_node *huffman_get_child1(struct linked_node *node);
struct linked_node *huffman_insert(struct linked_node *node, struct linked_node *other);
struct linked_node *huffman_decode(struct bit_reader *bit_reader, struct linked_node *head);
struct linked_node *huffman_build_list(uint16_t *prime_table);
struct linked_node *huffman_insert_node(struct linked_node *tail, int decomp);
void                huffman_adjust_tree(struct linked_node *new_node);
struct linked_node *huffman_build_tree(struct linked_node *tail);
void                huffman_free_linked_node(struct linked_node *node);
uint8_t            *huffman_decompress(uint8_t *buffer, uint32_t buffer_len, uint32_t *result_size);
#endif // ABYSS_HUFFMAN_H
