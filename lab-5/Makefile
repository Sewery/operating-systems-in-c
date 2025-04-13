CC=gcc
CFLAGS=-Wall
all: zad1 zad2sender zad2catcher
zad1: zad1.c
	$(CC) $(CFLAGS) zad1.c -o zad1
zad2sender: zad2sender.c
	$(CC) $(CFLAGS) zad2sender.c -o zad2sender
zad2catcher: zad2catcher.c
	$(CC) $(CFLAGS) zad2catcher.c -o zad2catcher		
pack: clean
	tar cvzf ./SewerynTasior-cw05.tar.gz ../lab-5
clean: 
	rm -f *.o zad1 zad2sender zad2catcher *.tar.gz