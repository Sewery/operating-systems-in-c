#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h> 
int main(int lpar, char *tab[]){
      int w1; // Deskryptor pliku dla potoku

    mkfifo("potok1", S_IFIFO | S_IRWXU); // Tworzy nazwany potok (FIFO) o nazwie "potok1".
                                     // S_IFIFO określa, że jest to FIFO.
                                     // 0666 nadaje prawa odczytu i zapisu dla właściciela, grupy i innych.
                                     // Uwaga: Jeśli potok już istnieje, mkfifo zwróci błąd (ale nie przerwie programu).

    w1 = open("potok1", O_WRONLY); // Otwiera potok "potok1" w trybie tylko do zapisu (blokujący).
                                     // Program zatrzyma się tutaj, dopóki inny proces nie otworzy tego samego potoku do odczytu.
    write(w1, "123456789\n", 12); // Zapisuje ciąg znaków "123456789" wraz ze znakiem nowej linii do potoku.
                                  // write zwraca liczbę zapisanych bajtów (tutaj oczekiwane 12).
    close(w1); // Zamyka deskryptor pliku potoku po zapisaniu danych.
    return 0;
}