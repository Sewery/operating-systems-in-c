SOURCE_DIR=./lab-3/art
OUTPUT_DIR=./lab-3/dir-out 
CC=gcc
CFLAGS=-Wall -std=c17 -g 
all: flipper
flipper: flipper.c
	$(CC) $(CFLAGS) flipper.c -o flipper
pack: all
	tar cvzf ./SewerynTasior-cw03.tar.gz ../lab-3
clean: 
	rm -f *.o flipper *.tar.gz