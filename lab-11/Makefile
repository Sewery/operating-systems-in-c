CC=gcc
CFLAGS=-g -Wall
all: klient serwer
klient: klient.c
	$(CC) $(CFLAGS) klient.c -o klient
serwer: serwer.c
	$(CC) $(CFLAGS) serwer.c -o serwer
pack: clean
	tar cvzf ./SewerynTasior-cw10.tar.gz ../lab-10
clean: 
	rm -f *.o klient serwer *.tar.gz