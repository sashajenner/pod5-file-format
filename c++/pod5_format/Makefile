CC = cc
CFLAGS += -Wall -O2 -std=c99 -march=native -fpic -g

.PHONY = clean

signal_compression.o: signal_compression.c signal_compression.h svb16/*
	$(CC) $(CFLAGS) $< -c -fpic -o $@

clean:
	rm -f signal_compression.o
