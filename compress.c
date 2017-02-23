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

    // print list of symbol frequencies
    queue_debug_print(queue);

    // build Huffman tree
    node_t *tree = tree_construct(queue);

    // print codes for the tree
    tree_debug_print(tree, 0, 0x00);

    // write compressed data to file
    FILE *out = fopen("data.huff", "wb");
    list_t *Rover;

    // First, write the frequency table, starting with the number of entries.
    fwrite(&num_in_list, sizeof(int), 1, out);
    Rover = head;
    while (Rover != NULL) {
      fwrite(&(Rover->symbol), sizeof(unsigned char), 1, out);
      fwrite(&(Rover->frequency), sizeof(int), 1, out);
      Rover = Rover->next;
    }

    // Next, we need to write the compressed data, starting with the total number of bits.
    int total_bits = 0;

    for ( j = 0; j < data_len; j++ ) {
      total_bits += getCodeLength(tree, 0, 0x00, data[j]);
      if (total_bits == 0) {
        printf("\nError in getCodeLength! (Returned a zero)\n\n");
        exit(1);
      }
    }

    fwrite(&data_len, sizeof(long), 1, out);

    // The number of bits modulo (write size) will matter when we get to the end of our data.
    unsigned long int buffer; // 32 bits
    int bits_in_buffer = 0;
    int current_length;
    int difference;
    unsigned long int current_code;
    unsigned long int temp;

    for ( j = 0; j < data_len; j++ ) {
      current_length = getCodeLength(tree, 0, 0x00, data[j]);
      current_code = getCode(tree, 0, 0x00, data[j]);

      if (bits_in_buffer == 0) {
            buffer |= current_code;
            bits_in_buffer = current_length;
      } else if (bits_in_buffer + current_length == (sizeof(unsigned long int) * 8)) {
            buffer <<= current_length;
            buffer |= current_code;
            fwrite(&buffer, sizeof(unsigned long int), 1, out);
            buffer &= 0;
            bits_in_buffer = 0;
      } else if (bits_in_buffer + current_length > (sizeof(unsigned long int) * 8)) {
        // Case where we need to stuff the remaining bits that will fit, write the buffer, flush the buffer, and then stuff the leftover bits.
            difference = (sizeof(unsigned long int) * 8) - bits_in_buffer;
            temp &= 0;
            temp |= current_code;
            temp >>= (current_length - difference);
            buffer <<= difference;
            buffer |= temp;
            fwrite(&buffer, sizeof(unsigned long int), 1, out);
            buffer &= 0;
            temp &= 0;
            temp |= current_code;
            switch (current_length - difference) {
              case 1:
                temp &= 0x1;
                break;
              case 2:
                temp &= 0x3;
                break;
              case 3:
                temp &= 0x7;
                break;
              case 4:
                temp &= 0xF;
                break;
              case 5:
                temp &= 0x1F;
                break;
              case 6:
                temp &= 0x3F;
                break;
              case 7:
                temp &= 0x7F;
                break;
              case 8:
                temp &= 0xFF;
                break;
              case 9:
                temp &= 0x1FF;
                break;
              case 10:
                temp &= 0x3FF;
                break;
              case 11:
                temp &= 0x7FF;
                break;
              case 12:
                temp &= 0xFFF;
                break;
              case 13:
                temp &= 0x1FFF;
                break;
              case 14:
                temp &= 0x3FFF;
                break;
              case 15:
                temp &= 0x7FFF;
                break;
              case 16:
                temp &= 0xFFFF;
                break;
              default:
                printf("\nBuffer overflow error while writing compresseed data!\n\n");
                exit(1);
            }
            buffer |= temp;
            bits_in_buffer = (current_length - difference);
      } else {
        // Case where we can just stuff the bits into the buffer.
            buffer <<= current_length;
            buffer |= current_code;
            bits_in_buffer += current_length;
      }
    }

    if (bits_in_buffer > 0) {
      buffer <<= ((sizeof(unsigned long int) * 8) - bits_in_buffer);
      fwrite(&buffer, sizeof(unsigned long int), 1, out);
    }

    fclose(out);

    // cleanup
    free(data);
    queue_destruct(queue);
    tree_destruct(tree);

    return 0;
}
