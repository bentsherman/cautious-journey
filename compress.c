/**
 * @file compress.c
 *
 * Implementation of Huffman compression.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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
 * Print the Huffman code for each leaf node in a binary tree.
 *
 * As a common convention, bit '0' represents following the
 * left child and bit '1' represents following the right child.
 *
 * @param root
 * @param len
 * @param code
 */
void huffman_print(node_t *root, int len, unsigned int code)
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
        huffman_print(root->left, len + 1, (code << 1) | 0x0);
        huffman_print(root->right, len + 1, (code << 1) | 0x1);
    }
}

int main(int argc, char **argv)
{
    // parse command-line arguments
    if ( argc != 2 ) {
        fprintf(stderr, "usage: ./compress [filename]\n");
        return 1;
    }

    // read file
    FILE *in = fopen(argv[1], "rb");

    fseek(in, 0L, SEEK_END);

    long data_len = ftell(in);
    char *data = (char *)malloc(data_len * sizeof(char));

    fseek(in, 0L, SEEK_SET);
    fread(data, sizeof(char), data_len, in);
    fclose(in);

    // create a priority queue of symbols
    queue_t *queue = queue_construct(8);

    int i, j;
    for ( i = 0; i < 256; i++ ) {
        char symbol = (char) i;
        int weight = 0;

        for ( j = 0; j < data_len; j++ ) {
            if ( data[j] == symbol ) {
                weight++;
            }
        }

        if ( weight > 0 ) {
            node_t *entry = (node_t *)malloc(sizeof(node_t));
            entry->symbol = symbol;
            entry->weight = weight;
            entry->left = NULL;
            entry->right = NULL;

            queue_push(queue, entry);
        }
    }

    queue_debug_print(queue);

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

    // print codes for the tree
    huffman_print(tree, 0, 0x00);

    // write compressed data to file
    FILE *out = fopen("data.huff", "wb");

    // ...

    fclose(out);

    // cleanup
    free(data);
    queue_destruct(queue);
    tree_destruct(tree);

    return 0;
}
