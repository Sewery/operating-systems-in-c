#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int lpar, char *tab[]){
    pid_t pid; // Zmienna do przechowywania identyfikatora procesu (PID)
    int fd[2], w; // fd to tablica dwóch deskryptorów plików dla potoku, w będzie przechowywać liczbę zapisanych/odczytanych bajtów
    char buf[256]; // Bufor do przechowywania danych odczytanych z potoku

    pipe(fd); // Tworzy potok nienazwany. fd[0] to koniec do odczytu, fd[1] to koniec do zapisu

    pid = fork(); // Tworzy nowy proces potomny

    if(pid == 0){ // Kod wykonywany przez proces potomny
        close(fd[0]); // Potomek nie będzie czytać z potoku, więc zamyka koniec do odczytu

        w = write(fd[1], "123456789", 10); // Potomek zapisuje 10 bajtów (ciąg "123456789" + null terminator, ale określono 10 bajtów) do potoku
        // Uwaga: write zwraca liczbę faktycznie zapisanych bajtów, która może być mniejsza niż żądana w przypadku błędu lub pełnego potoku

        sleep(5); // Potomek usypia na 5 sekund

        close(fd[1]); // Potomek zamyka koniec do zapisu potoku. Jest to ważne, aby zasygnalizować koniec strumienia danych dla procesu macierzystego
        return 0; // Potomek kończy swoje działanie
    }

    // Kod wykonywany przez proces macierzysty (pid > 0)
    close(fd[1]); // Rodzic nie będzie pisać do potoku, więc zamyka koniec do zapisu

    w = read(fd[0], buf, 10); // Rodzic próbuje odczytać maksymalnie 10 bajtów z potoku do bufora 'buf'
    // Uwaga: read zwróci liczbę faktycznie odczytanych bajtów. Jeśli potok jest pusty i żaden proces nie ma otwartego końca do zapisu, read zwróci 0 (EOF)

    close(fd[0]); // Rodzic zamyka koniec do odczytu potoku

    buf[w] = 0; // Dodaje znak null na końcu odczytanych danych, aby potraktować je jako ciąg znaków C
    // Ważne: Jeśli read odczytał 0 bajtów (potok był pusty), to buf[0] = 0;

    printf("%s\n", buf); // Wypisuje odczytane dane na standardowe wyjście
    fflush(stdout); // Wymusza natychmiastowe wypisanie bufora standardowego wyjścia

    return 0; // Rodzic kończy swoje działanie
}