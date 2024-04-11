#ifndef ABYSS_HUFFMAN_H
#define ABYSS_HUFFMAN_H

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
struct linked_node *huffman_insert_node(struct linked_node *node, struct linked_node *other);

#endif // ABYSS_HUFFMAN_H
