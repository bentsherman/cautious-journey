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

    // write decompressed data to file
    FILE *out = fopen("data.uhuff", "wb");

    // We need to read in the total number of bits.
    long codeCount;
    if (fread(&codeCount, sizeof(long), 1, in) == 0) {
      printf("\nThe total number of bits wasn't read! Check format of compressed file.\n\n");
      exit(1);
    }

    unsigned int buffer;
    unsigned int next;
    int bits_in_next = sizeof(unsigned int) * 8;

    fread(&buffer, sizeof(unsigned int), 1, in);
    fread(&next, sizeof(unsigned int), 1, in);

    int j;
    for (j = 0; j < codeCount; j++) {
      char symbol = getSymbol(tree, buffer);
      int codeLength = getCodeLength(tree, 0, 0x00, symbol);

      buffer <<= codeLength;

      if ( bits_in_next >= codeLength ) {
        buffer |= (next >> ((sizeof(unsigned int) * 8) - codeLength));

        next <<= codeLength;
        bits_in_next -= codeLength;
      }
      else {
        // empty next
        buffer |= (next >> ((sizeof(unsigned int) * 8) - codeLength));

        int difference = codeLength - bits_in_next;

        next <<= codeLength;
        bits_in_next = 0;

        // fill up next
        fread(&next, sizeof(unsigned int), 1, in);
        bits_in_next = sizeof(unsigned int) * 8;

        // fill up buffer
        buffer |= (next >> ((sizeof(unsigned int) * 8) - difference));

        next <<= difference;
        bits_in_next -= difference;
      }

      fwrite(&symbol, sizeof(char), 1, out);
    }

    fclose(in);
    fclose(out);

    return 0;
}
