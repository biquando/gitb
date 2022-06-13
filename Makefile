CC = gcc
CFLAGS = -Wall

.PHONY: clean

bin/gitb: gitb.c
	mkdir -p bin
	$(CC) -o $@ $(CFLAGS) $^

clean:
	rm -f bin/*
	rm -df bin
