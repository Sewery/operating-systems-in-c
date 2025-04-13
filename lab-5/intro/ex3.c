#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *tab[]){
    if (argc != 2) {
        fprintf(stderr, "Użycie: %s <PID>\n", tab[0]);
        return 1;
    }

    union sigval war;
    int pid = atoi(tab[1]);

    // Sprawdzenie, czy PID jest poprawny (większy od 0).
    if (pid <= 0) {
        fprintf(stderr, "Nieprawidłowy PID: %s\n", tab[1]);
        return 1;
    }

    war.sival_int = 123;

    // Wysyłanie sygnału SIGUSR1 z dodatkową wartością do procesu o podanym PID.
    if (sigqueue(pid, SIGUSR1, war) == -1) {
        perror("sigqueue");
        return 1;
    }

    return 0;
}