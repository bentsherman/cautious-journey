/**
 * @file compress.c
 *
 * Implementation of Huffman compression.
 */
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

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

    // print list of symbol frequencies
    queue_debug_print(queue);

    // build Huffman tree
    node_t *tree = tree_construct(queue);

    // print codes for the tree
    tree_debug_print(tree, 0, 0x00);

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
