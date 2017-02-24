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
        exit(1);
    }

    FILE *in = fopen(argv[1], "rb");

    // read symbol table
    queue_t *queue = queue_construct(8);

    int i;
    for ( i = 0; i < 256; i++ ) {
        int weight;

        fread(&weight, sizeof(int), 1, in);

        if ( weight > 0 ) {
            node_t *entry = (node_t *)malloc(sizeof(node_t));
            entry->symbol = (symbol_t) i;
            entry->weight = weight;
            entry->left = NULL;
            entry->right = NULL;

            queue_push(queue, entry);
        }
    }

    // build Huffman tree
    node_t *tree = tree_construct(queue);

    // build Huffman table
    code_t *table = (code_t *)calloc(256, sizeof(code_t));

    get_code_table(tree, table, 0, 0x00);

    // write decompressed data to file
    FILE *out = fopen("data.uhuff", "wb");

    long num_codes;
    fread(&num_codes, sizeof(long), 1, in);

    unsigned int buffer;
    unsigned int next;
    int buffer_size = sizeof(unsigned int) * 8;
    int bits_in_next = buffer_size;

    fread(&buffer, sizeof(unsigned int), 1, in);
    fread(&next, sizeof(unsigned int), 1, in);

    for ( i = 0; i < num_codes; i++ ) {
        symbol_t symbol = get_symbol(tree, buffer);
        int code_length = table[symbol].len;

        // shift current code out of buffer
        buffer <<= code_length;

        if ( bits_in_next >= code_length ) {
            // fill up buffer
            buffer |= (next >> (buffer_size - code_length));

            next <<= code_length;
            bits_in_next -= code_length;
        }
        else {
            int num_fill = code_length - bits_in_next;

            // empty next
            buffer |= (next >> (buffer_size - code_length));

            next <<= code_length;
            bits_in_next = 0;

            // fill up next
            fread(&next, sizeof(unsigned int), 1, in);
            bits_in_next = buffer_size;

            // fill up buffer
            buffer |= (next >> (buffer_size - num_fill));

            next <<= num_fill;
            bits_in_next -= num_fill;
        }

        // write symbol to file
        fwrite(&symbol, sizeof(symbol_t), 1, out);
    }

    fclose(in);
    fclose(out);

    return 0;
}
