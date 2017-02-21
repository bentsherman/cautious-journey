CC = gcc
CFLAGS = -g -Wall

BINS = compress decompress

all: $(BINS)

compress: compress.c tree.c
	$(CC) $(CFLAGS) -o $@ $^

decompress: decompress.c tree.c
	$(CC) $(CFLAGS) -o $@ $^

test: all
	./compress $(FILE)
	./decompress data.huff
	diff $(FILE) data.uhuff

clean:
	rm -f $(BINS) *.huff *.uhuff
