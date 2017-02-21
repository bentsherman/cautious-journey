/**
 * @file tree.h
 *
 * Interface definitions for the priority queue and
 * Huffman tree types.
 */
#ifndef TREE_H
#define TREE_H

typedef struct node {
    char symbol;
    int weight;
    struct node *left;
    struct node *right;
} node_t;

typedef struct {
    int size;
    int num_entries;
    node_t **entries;
} queue_t;

queue_t * queue_construct(int size);
void queue_destruct(queue_t *queue);
int queue_length(queue_t *queue);
void queue_push(queue_t *queue, node_t *entry);
node_t * queue_remove(queue_t *queue);
void queue_debug_print(queue_t *queue);

node_t * tree_construct(queue_t *queue);
void tree_destruct(node_t *root);
void tree_debug_print(node_t *root, int len, unsigned int code);

#endif
