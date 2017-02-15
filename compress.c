/**
 * @file compress.c
 *
 * Implementation of Huffman compression.
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char symbol;
    int weight;
} queue_entry_t;

typedef struct {
    int size;
    int num_entries;
    queue_entry_t *entries;
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
    queue->entries = (queue_entry_t *)malloc(size * sizeof(queue_entry_t));

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
 * Push an entry into a priority queue.
 *
 * @param queue
 * @param symbol
 * @param weight
 */
void queue_push(queue_t *queue, char symbol, int weight)
{
    // increase queue size if queue is full
    if ( queue->num_entries == queue->size ) {
        queue->size *= 2;
        queue->entries = (queue_entry_t *)realloc(queue->entries, queue->size * sizeof(queue_entry_t));
    }

    // insert entry
    queue_entry_t *entry = &queue->entries[queue->num_entries];
    entry->symbol = symbol;
    entry->weight = weight;

    queue->num_entries++;
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
        queue_entry_t *entry = &queue->entries[i];

        printf("%hhu  %d\n", entry->symbol, entry->weight);
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
            queue_push(queue, symbol, weight);
        }
    }

    queue_debug_print(queue);

    // write compressed data to file
    FILE *out = fopen("data.huff", "wb");

    // ...

    fclose(out);

    // cleanup
    free(data);
    queue_destruct(queue);

    return 0;
}
