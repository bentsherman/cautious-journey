CC = gcc
CFLAGS = -g -Wall

BINS = compress decompress

all: $(BINS)

compress: compress.c
	$(CC) $(CFLAGS) -o $@ $^

decompress: decompress.c
	$(CC) $(CFLAGS) -o $@ $^

test: all
	./compress $(FILE)
	./decompress data.huff
	diff $(FILE) data.uhuff

clean:
	rm $(BINS) *.huff *.uhuff
