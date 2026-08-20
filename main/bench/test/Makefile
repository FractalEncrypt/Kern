CC = gcc
CFLAGS = -Wall -Wextra -O2 -g

ROOT = ../../..
SLH = $(ROOT)/components/slhdsa-c/upstream

SRCS = test_kat.c $(SLH)/slh_dsa.c $(SLH)/slh_sha2.c $(SLH)/sha2_256.c $(SLH)/sha2_512.c

all: test_kat test_unwrap

test_kat: $(SRCS) $(ROOT)/main/bench/kat_slhdsa_sha2_128s.h
	$(CC) $(CFLAGS) -I$(SLH) -o $@ $(SRCS)

test_unwrap: test_unwrap.c $(ROOT)/main/bench/cycle_unwrap.h
	$(CC) $(CFLAGS) -o $@ test_unwrap.c

run: all
	./test_unwrap
	./test_kat

clean:
	rm -f test_kat test_unwrap

.PHONY: all run clean
