CC=gcc
CFLAGS=-Wall -std=c17

all: main_ladow_dynam main_stat main_wspoldz 
main_ladow_dynam: libbibl1.so main2.c
	$(CC) main2.c -o main_ladow_dynam -ldl
main_stat: libbibl1.a
	$(CC) main.c libbibl1.a -o main_stat
main_wspoldz: libbibl1.so
	$(CC) main.c -l bibl1 -L ./ -Wl,-rpath,./ -o main_wspoldz
libbibl1.a: bibl1.o 
	ar rcs libbibl1.a bibl1.o
bibl1.o:
	$(CC) -c bibl1.c
libbibl1.so: bibl1.o-wsp
	$(CC) -shared  bibl1.o -o libbibl1.so
bibl1.o-wsp:
	$(CC) -fPIC -c bibl1.c -o bibl1.o
.PHONY: clean all
pack: all
	tar cvzf ../SewerynTasior-cw02.tar.gz ../lab-2
clean:
	rm -f *.o *.a *.so main_ladow_dynam main_stat main_wspoldz
