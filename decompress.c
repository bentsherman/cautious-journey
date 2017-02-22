/**
 * @file decompress.c
 *
 * Implementation of Huffman decompression.
 */
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

int main(int argc, char **argv)
{
    // parse command-line arguments
    if ( argc != 2 ) {
        fprintf(stderr, "usage: ./decompress [filename]\n");
        return 1;
    }

    // read file
    FILE *in = fopen(argv[1], "rb");

    // We need to read in the frequency table first, and the data starts with the number of entries in the table.
    int total_entries;
    unsigned char symbol;
    int weight;
    int count = 0;

    // Read in the total number of entries.
    if (fread(&total_entries, sizeof(int), 1, in) == 0) {
      printf("\nCouldn't read data from file.\n\n");
      exit(1);
    }

    // Read in the frequency table.
    queue_t *queue = queue_construct(8);
    while (count < total_entries) {
      fread(&symbol, sizeof(unsigned char), 1, in);
      fread(&weight, sizeof(int), 1, in);

      node_t *entry = (node_t *)malloc(sizeof(node_t));
      entry->symbol = symbol;
      entry->weight = weight;
      entry->left = NULL;
      entry->right = NULL;

      queue_push(queue, entry);

      count++;
    }

    // build Huffman tree
    node_t *tree = tree_construct(queue);
    tree_debug_print(tree, 0, 0x00);

    /*
    fseek(in, 0L, SEEK_END);

    long data_len = ftell(in);
    char *data = (char *)malloc(data_len * sizeof(char));

    fseek(in, 0L, SEEK_SET);
    fread(data, sizeof(char), data_len, in);
    fclose(in);
    */

    // write decompressed data to file
    FILE *out = fopen("data.uhuff", "wb");

    // ...

    fclose(in);
    fclose(out);

    // cleanup
    //free(data);

    return 0;
}
