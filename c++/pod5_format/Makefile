# compilation options
# NO_SIMD=1 for SSE SIMD
# ZSTD=1 for zstd

CC = cc
CFLAGS += -Wall -O2 -std=c99 -fpic -g
ifeq ($(NO_SIMD),1)
else
CFLAGS += -march=native -DVBZ_ENABLE_SIMD
endif
ifeq ($(ZSTD),1)
CFLAGS += -DVBZ_USE_ZSTD
LDFLAGS += -lzstd
endif

.PHONY = clean

signal_compression.o: signal_compression.c signal_compression.h svb16/*
	$(CC) $(CFLAGS) $< -c -fpic -o $@ $(LDFLAGS)

clean:
	rm -f signal_compression.o
