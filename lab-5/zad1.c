#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <bits/sigaction.h>
#include <bits/types/sigset_t.h>

void handler(int sig) {
    printf("Odebrano sygnał SIGUSR1.\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Niepoprawne wejście \n");
        return 1;
    }

    char *option = argv[1];
    sigset_t mask, oldmask, set;

    if (strcmp(option, "none") == 0) {
    }

    else if (strcmp(option, "ignore") == 0) {
        signal(SIGUSR1, SIG_IGN);
    }

    else if (strcmp(option, "handler") == 0) {
        signal(SIGUSR1, handler);
    }

    else if (strcmp(option, "mask") == 0) {
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &mask, &oldmask);  // blokujemy SIGUSR1
    }

    else {
        fprintf(stderr, "Niepoprawna opcja: %s\n", option);
        return 1;
    }

    raise(SIGUSR1);

    if (strcmp(option, "mask") == 0) {
        sigpending(&set);
        if (sigismember(&set, SIGUSR1)) {
            printf("Sygnał SIGUSR1 jest w kolejce oczekujących.\n");
        } else {
            printf("Brak oczekujących sygnałów SIGUSR1.\n");
        }

        sigprocmask(SIG_SETMASK, &oldmask, NULL);
    }

    return 0;
}
