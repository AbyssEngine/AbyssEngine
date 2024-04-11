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
struct linked_node *huffman_insert(struct linked_node *node, struct linked_node *other);
struct linked_node *huffman_decode(char *buffer, struct linked_node *head);

#endif // ABYSS_HUFFMAN_H
