CC=gcc
CFLAGS=-g -Wall
all: user printer 
user: user.c
	$(CC) $(CFLAGS) user.c -o user
printer: printer.c
	$(CC) $(CFLAGS) printer.c -o printer	
pack: clean
	tar cvzf ./SewerynTasior-cw08.tar.gz ../lab-8
clean: 
	rm -f *.o zad printer *.tar.gz