CC=gcc
CFLAGS=-Wall -lpthread -ggdb
all: zad
zad: zad.c
	$(CC) $(CFLAGS) zad.c -o zad
pack: clean
	tar cvzf ./SewerynTasior-cw10.tar.gz ../lab-10
clean: 
	rm -f *.o zad *.tar.gz