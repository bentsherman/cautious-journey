/**
 * @file tree.c
 *
 * Implementation of the Huffman tree type.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

/**
 * Construct a priority queue.
 *
 * @param size
 */
queue_t * queue_construct(int size)
{
    queue_t *queue = (queue_t *)malloc(sizeof(queue_t));
    queue->size = size;
    queue->num_entries = 0;
    queue->entries = (node_t **)malloc(size * sizeof(node_t *));

    return queue;
}

/**
 * Destruct a priority queue.
 *
 * @param queue
 */
void queue_destruct(queue_t *queue)
{
    free(queue->entries);
    free(queue);
}

/**
 * Get the length of a priority queue.
 *
 * @param queue
 * @return length of queue
 */
int queue_length(queue_t *queue)
{
    return queue->num_entries;
}

/**
 * Push an entry into a priority queue.
 *
 * @param queue
 * @param entry
 */
void queue_push(queue_t *queue, node_t *entry)
{
    // increase queue size if queue is full
    if ( queue->num_entries == queue->size ) {
        queue->size *= 2;
        queue->entries = (node_t **)realloc(queue->entries, queue->size * sizeof(node_t *));
    }

    // insert entry
    queue->entries[queue->num_entries] = entry;
    queue->num_entries++;
}

/**
 * Remove the entry of highest priority from a queue.
 *
 * The entry of highest priority is the entry with the
 * lowest weight.
 *
 * @param queue
 * @return entry of highest priority, or NULL if queue is empty
 */
node_t * queue_remove(queue_t *queue)
{
    if ( queue->num_entries == 0 ) {
        return NULL;
    }

    // find entry with the lowest weight
    int min_index = 0;
    node_t *min = queue->entries[0];

    int i;
    for ( i = 1; i < queue->num_entries; i++ ) {
        node_t *entry = queue->entries[i];

        if ( min->weight > entry->weight ) {
            min_index = i;
            min = entry;
        }
    }

    // remove the entry from the queue
    for ( i = min_index; i + 1 < queue->num_entries; i++ ) {
        queue->entries[i] = queue->entries[i + 1];
    }

    queue->num_entries--;

    return min;
}

/**
 * Print a priority queue.
 *
 * @param queue
 */
void queue_debug_print(queue_t *queue)
{
    int i;
    for ( i = 0; i < queue->num_entries; i++ ) {
        node_t *entry = queue->entries[i];

        printf("%3hhu  %8d\n", entry->symbol, entry->weight);
    }
    putchar('\n');
}

/**
 * Build a Huffman tree.
 */
node_t * tree_construct(queue_t *queue)
{
    // create binary tree
    while ( queue_length(queue) > 1 ) {
        // remove the two nodes of lowest weight
        node_t *left = queue_remove(queue);
        node_t *right = queue_remove(queue);

        // create a parent node whose weight is the sum of the children
        node_t *parent = (node_t *)malloc(sizeof(node_t));
        parent->weight = left->weight + right->weight;
        parent->left = left;
        parent->right = right;

        // add parent node to the queue
        queue_push(queue, parent);
    }

    // set the last node as the root node
    node_t *tree = queue_remove(queue);

    return tree;
}

/**
 * Destruct a binary tree.
 *
 * @param root
 */
void tree_destruct(node_t *root)
{
    if ( root == NULL ) {
        return;
    }

    free(root->left);
    free(root->right);
    free(root);
}

/**
 * Print the Huffman code for each leaf node in a Huffman tree.
 *
 * As a common convention, bit '0' represents following the
 * left child and bit '1' represents following the right child.
 *
 * @param root
 * @param len
 * @param code
 */
void tree_debug_print(node_t *root, int len, code_t code)
{
    assert((root->left == NULL) == (root->right == NULL));

    if ( root->left == NULL && root->right == NULL ) {
        printf("%3hhu  %8d  %2d  ", root->symbol, root->weight, len);

        int i;
        for ( i = 0; i < len; i++ ) {
            char digit = (((code >> (len - i - 1)) & 0x1) == 0x1)
                ? '1'
                : '0';

            putchar(digit);
        }

        putchar('\n');
    }
    else {
        tree_debug_print(root->left, len + 1, (code << 1) | 0x0);
        tree_debug_print(root->right, len + 1, (code << 1) | 0x1);
    }
}

/**
 * Return the Huffman code length for a specific symbol in the tree.
 *
 *
 * @param root
 * @param len
 * @param code
 * @param symbol
 */
int getCodeLength(node_t *root, int len, code_t code, symbol_t symbol)
{
    assert((root->left == NULL) == (root->right == NULL));

    if ( root->left == NULL && root->right == NULL) {
        return (root->symbol == symbol)
            ? len
            : 0;
    }

    return getCodeLength(root->left, len + 1, (code << 1) | 0x0, symbol) + getCodeLength(root->right, len + 1, (code << 1) | 0x1, symbol);
}

/**
 * Return the Huffman code for a specific symbol in the tree.
 *
 *
 * @param root
 * @param len
 * @param code
 * @param symbol
 */
code_t getCode(node_t *root, int len, code_t code, symbol_t symbol)
{
    assert((root->left == NULL) == (root->right == NULL));

    if ( root->left == NULL && root->right == NULL ) {
        return (root->symbol == symbol)
            ? code
            : 0;
    }

    return getCode(root->left, len + 1, (code << 1) | 0x0, symbol) + getCode(root->right, len + 1, (code << 1) | 0x1, symbol);
}

/**
 * Return the symbol for a Huffman code in the tree.
 *
 * @param root
 * @param buffer
 */
symbol_t getSymbol(node_t *root, unsigned int buffer)
{
    assert((root->left == NULL) == (root->right == NULL));

    if ( root->left == NULL && root->right == NULL ) {
        return root->symbol;
    }

    node_t *child = (buffer & 0x80000000)
      ? root->right
      : root->left;

    return getSymbol(child, buffer << 1);
}

/**
 * Adds a symbol frequency pair to the list.
 *
 * @param head
 * @param tail
 * @param symbol
 * @param frequency
 */
list_t *listAdd(list_t *head, list_t *tail, symbol_t symbol, int frequency) {
  list_t *entry;

    entry = (list_t *)malloc(sizeof(list_t));
    entry->symbol = symbol;
    entry->frequency = frequency;
    entry->next = NULL;
    if (head != NULL) {
      tail->next = entry;
    } else {
      head = entry;
    }
    tail = entry;
    return tail;
}
