/**
 * @file compress.c
 *
 * Implementation of Huffman compression.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

int main(int argc, char **argv)
{
    // parse command-line arguments
    if ( argc != 2 ) {
        fprintf(stderr, "usage: ./compress [filename]\n");
        exit(1);
    }

    // read file
    FILE *in = fopen(argv[1], "rb");

    fseek(in, 0L, SEEK_END);

    long data_len = ftell(in);
    symbol_t *data = (symbol_t *)malloc(data_len * sizeof(symbol_t));

    fseek(in, 0L, SEEK_SET);
    fread(data, sizeof(symbol_t), data_len, in);
    fclose(in);

    // compute the symbol weights
    int weights[256];
    memset(weights, 0, 256 * sizeof(int));

    int i;
    for ( i = 0; i < data_len; i++ ) {
        weights[data[i]]++;
    }

    // create a priority queue of symbol-weight pairs
    queue_t *queue = queue_construct(8);

    for ( i = 0; i < 256; i++ ) {
        if ( weights[i] > 0 ) {
            node_t *entry = (node_t *)malloc(sizeof(node_t));
            entry->symbol = (symbol_t) i;
            entry->weight = weights[i];
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

    // write symbol table to file
    FILE *out = fopen("data.huff", "wb");

    for ( i = 0; i < 256; i++ ) {
        fwrite(&weights[i], sizeof(int), 1, out);
    }

    // write compressed data to file
    unsigned int buffer;
    int buffer_size = sizeof(unsigned int) * 8;
    int bits_in_buffer = 0;

    fwrite(&data_len, sizeof(long), 1, out);

    for ( i = 0; i < data_len; i++ ) {
        code_t code = table[data[i]];

        if ( bits_in_buffer + code.len < buffer_size ) {
            // shift code into buffer
            buffer <<= code.len;
            buffer |= code.code;

            bits_in_buffer += code.len;
        }
        else if ( bits_in_buffer + code.len == buffer_size ) {
            // shift code into buffer
            buffer <<= code.len;
            buffer |= code.code;

            // write buffer to file
            fwrite(&buffer, sizeof(unsigned int), 1, out);

            // empty buffer
            buffer = 0;
            bits_in_buffer = 0;
        }
        else if ( bits_in_buffer + code.len > buffer_size ) {
            // fill buffer
            int num_fill = buffer_size - bits_in_buffer;
            int num_overflow = code.len - num_fill;

            buffer <<= num_fill;
            buffer |= (code.code >> num_overflow);

            // write buffer to file
            fwrite(&buffer, sizeof(unsigned int), 1, out);

            // empty buffer
            buffer = 0;
            bits_in_buffer = 0;

            // shift overflow bits into buffer
            buffer = code.code & ~(0xFFFFFFFF << num_overflow);
            bits_in_buffer = num_overflow;
        }
    }

    // write remaining bits in buffer
    if ( bits_in_buffer > 0 ) {
        buffer <<= buffer_size - bits_in_buffer;
        fwrite(&buffer, sizeof(unsigned int), 1, out);
    }

    fclose(out);

    // cleanup
    free(data);
    queue_destruct(queue);
    tree_destruct(tree);
    free(table);

    return 0;
}
