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

    fseek(in, 0L, SEEK_END);

    long data_len = ftell(in);
    char *data = (char *)malloc(data_len * sizeof(char));

    fseek(in, 0L, SEEK_SET);
    fread(data, sizeof(char), data_len, in);
    fclose(in);

    // read list of word frequencies into priority queue

    // build Huffman tree

    // write decompressed data to file
    FILE *out = fopen("data.uhuff", "wb");

    // ...

    fclose(out);

    // cleanup
    free(data);

    return 0;
}
