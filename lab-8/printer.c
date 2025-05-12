#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>

#define QUEUE_SIZE 10
#define TASK_SIZE 10

typedef struct {
    char queue[QUEUE_SIZE][TASK_SIZE];
    int head;
    int tail;
    int count;
} shared_queue;

int main(int argc, char *argv[]) {
    // if (argc != 2) {
    //     perror( "Podano złe arugmenty");
    //     exit(EXIT_FAILURE);
    // }
    // int mode = atoi(argv[1]); // Tryb

    // Tworzenie pamięci współdzielonej
    int shm_fd = shm_open("/print_queue", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    ftruncate(shm_fd, sizeof(shared_queue));
    shared_queue *shm = mmap(NULL, sizeof(shared_queue), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    shm->head = 0;
    shm->tail = 0;
    shm->count = 0;

    sem_t *sem_queue = sem_open("/sem_queue", O_CREAT, 0666, 1); // Dostęp do kolejki
    sem_t *sem_empty = sem_open("/sem_empty", O_CREAT, 0666, QUEUE_SIZE); // Wolne miejsca w kolejce
    sem_t *sem_full = sem_open("/sem_full", O_CREAT, 0666, 0); // Zadania w kolejce

    while (1) {
        sem_wait(sem_full); // Czekaj na zadanie w kolejce
        sem_wait(sem_queue); // Zablokuj dostęp do kolejki

        // Pobierz zadanie z kolejki
        char task[TASK_SIZE];
        memcpy(task, shm->queue[shm->head], TASK_SIZE);
        shm->head = (shm->head + 1) % QUEUE_SIZE;
        shm->count--;

        sem_post(sem_queue); // Odblokuj dostęp do kolejki
        sem_post(sem_empty); // Zwiększ liczbę wolnych miejsc w kolejce

        printf("Drukarka %d drukuje zadanie: %.*s\n", getpid(), TASK_SIZE, task);
        for (int j = 0; j < TASK_SIZE; j++) {
            printf("%c", task[j]);
            fflush(stdout);
            sleep(1); // Symulacja wydruku
        }
        printf("\n");
    }

    // Sprzątanie
    sem_close(sem_queue);
    sem_close(sem_empty);
    sem_close(sem_full);
    sem_unlink("/sem_queue");
    sem_unlink("/sem_empty");
    sem_unlink("/sem_full");
    munmap(shm, sizeof(shared_queue));
    shm_unlink("/print_queue");

    return 0;
}