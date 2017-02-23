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

    // create a priority queue and a list of symbols
    queue_t *queue = queue_construct(8);
    list_t *head, *tail;
    int num_in_list = 0;
    head = NULL;
    tail = NULL;

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

            if (head == NULL) {
              head = listAdd(head, tail, symbol, weight);
              tail = head;
            }
            else
              tail = listAdd(head, tail, symbol, weight);

            num_in_list++;

            queue_push(queue, entry);
        }
    }

    // build Huffman tree
    node_t *tree = tree_construct(queue);

    // write compressed data to file
    FILE *out = fopen("data.huff", "wb");
    list_t *Rover;

    // First, write the frequency table, starting with the number of entries.
    fwrite(&num_in_list, sizeof(int), 1, out);
    Rover = head;
    while (Rover != NULL) {
      fwrite(&(Rover->symbol), sizeof(symbol_t), 1, out);
      fwrite(&(Rover->frequency), sizeof(int), 1, out);
      Rover = Rover->next;
    }

    // Next, we need to write the compressed data, starting with the total number of codes
    unsigned int buffer;
    int buffer_size = sizeof(unsigned int) * 8;
    int bits_in_buffer = 0;

    fwrite(&data_len, sizeof(long), 1, out);

    for ( j = 0; j < data_len; j++ ) {
        int current_length = getCodeLength(tree, 0, 0x00, data[j]);
        code_t current_code = getCode(tree, 0, 0x00, data[j]);

        if ( bits_in_buffer + current_length < buffer_size ) {
            // shift code into buffer
            buffer <<= current_length;
            buffer |= current_code;

            bits_in_buffer += current_length;
        }
        else if ( bits_in_buffer + current_length == buffer_size ) {
            // shift code into buffer
            buffer <<= current_length;
            buffer |= current_code;

            // write buffer to file
            fwrite(&buffer, sizeof(unsigned int), 1, out);

            // empty buffer
            buffer = 0;
            bits_in_buffer = 0;
        }
        else if ( bits_in_buffer + current_length > buffer_size ) {
            // fill buffer
            int num_fill = buffer_size - bits_in_buffer;
            int num_overflow = current_length - num_fill;

            buffer <<= num_fill;
            buffer |= (current_code >> num_overflow);

            // write buffer to file
            fwrite(&buffer, sizeof(unsigned int), 1, out);

            // empty buffer
            buffer = 0;
            bits_in_buffer = 0;

            // shift overflow bits into buffer
            buffer = current_code & ((unsigned int)(0xFFFFFFFF) >> (buffer_size - num_overflow));
            bits_in_buffer = num_overflow;
        }
    }

    if ( bits_in_buffer > 0 ) {
        buffer <<= buffer_size - bits_in_buffer;
        fwrite(&buffer, sizeof(unsigned int), 1, out);
    }

    fclose(out);

    // cleanup
    free(data);
    queue_destruct(queue);
    tree_destruct(tree);

    return 0;
}
