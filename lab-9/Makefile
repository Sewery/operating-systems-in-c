CC=gcc
CFLAGS=-Wall -lpthread
all: zad
zad: zad.c
	$(CC) $(CFLAGS) zad.c -o zad
pack: clean
	tar cvzf ./SewerynTasior-cw09.tar.gz ../lab-9
clean: 
	rm -f *.o zad *.tar.gz