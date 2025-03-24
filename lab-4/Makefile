CC=gcc
CFLAGS=-Wall -std=c17 -g 
all: zad1 zad2
zad1: zad1.c
	$(CC) $(CFLAGS) zad1.c -o zad1
zad2: zad2.c
	$(CC) $(CFLAGS) zad2.c -o zad2	
pack: clean
	tar cvzf ./SewerynTasior-cw04.tar.gz ../lab-4
clean: 
	rm -f *.o zad1 *.tar.gz