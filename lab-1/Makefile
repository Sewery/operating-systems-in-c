CC=gcc
CFLAGS=-Wall -std=c17 -g 
all: countdown
countdown: countdown.o
	$(CC) countdown.o -o countdown

countdown.o: countdown.c
	$(CC) $(CFLAGS) -c countdown.c -o countdown.o

.PHONY: clean
clean:
	rm -f countdown.o countdown