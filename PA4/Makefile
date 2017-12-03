#Makefile for PA4, pretty self explanitory

CC           = gcc

CFLAGSFUSE   = `pkg-config fuse --cflags`
LLIBSFUSE    = `pkg-config fuse --libs`
LLIBSOPENSSL = -lcrypto

CFLAGS = -c -g -Wall -Wextra -D_FILE_OFFSET_BITS=64
LFLAGS = -g -Wall -Wextra

.PHONY: all clean

all: pa4-encfs

pa4-encfs : pa4-encfs.o aes-crypt.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSFUSE) $(LLIBSOPENSSL)

pa4-encfs.o: pa4-encfs.c
	$(CC) $(CFLAGS) $(CFLAGSFUSE) $<

aes-crypt.o: aes-crypt.c aes-crypt.h
	$(CC) $(CFLAGS) $<

clean:
	rm -f pa4-encfs
	rm -f *.o
	rm -f *~
