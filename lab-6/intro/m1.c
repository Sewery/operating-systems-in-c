#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h> 
int main(int lpar, char *tab[]){
    int w1; // Deskryptor pliku dla potoku
    char buf[1024]; // Bufor do przechowywania danych odczytanych z potoku

    // mkfifo("potok1",S_ISFIFO | S_IRXWU); // Zakomentowano, ponieważ potok powinien być utworzony tylko raz przez jeden z procesów.

    w1 = open("potok1", O_RDONLY); // Otwiera potok "potok1" w trybie tylko do odczytu (blokujący).
                                     // Program zatrzyma się tutaj, dopóki inny proces nie otworzy tego samego potoku do zapisu i coś do niego nie zapisze.
    read(w1, buf, 12); // Odczytuje maksymalnie 12 bajtów z potoku do bufora 'buf'.
                       // Funkcja read zablokuje proces, dopóki jakieś dane nie będą dostępne w potoku.
    printf("otrzymano: %s\n", buf); // Wypisuje odczytane dane na standardowe wyjście.
    close(w1); // Zamyka deskryptor pliku potoku po odczytaniu danych.
    return 0;
}
