/**
 * @file compress.c
 *
 * Implementation of Huffman compression.
 */
#include <stdio.h>
#include <stdlib.h>

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

    // perform Huffman compression on data
    long huffman_len;
    char *huffman_data;

    // write compressed data to file
    FILE *out = fopen("data.huff", "wb");

    fwrite(huffman_data, sizeof(char), huffman_len, out);
    fclose(out);

    // cleanup
    free(data);
    free(huffman_data);

    return 0;
}
