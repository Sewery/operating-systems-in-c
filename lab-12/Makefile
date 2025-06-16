CC=gcc
CFLAGS=-g -Wall -lpthread
all: klient serwer
klient: klient.c
	$(CC) $(CFLAGS) klient.c -o klient
serwer: serwer.c
	$(CC) $(CFLAGS) serwer.c -o serwer
pack: clean
	tar cvzf ./SewerynTasior-cw12.tar.gz ../lab-12
clean: 
	rm -f *.o klient serwer *.tar.gz