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
    if (argc != 3) {
        perror( "Podano złe arugmenty");
        exit(EXIT_FAILURE);
    }
    int N = atoi(argv[1]); // Liczba użytkowników
    int M = atoi(argv[2]); // Liczba drukarek

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

    if (sem_queue == SEM_FAILED || sem_empty == SEM_FAILED || sem_full == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < N; i++) {
        if (fork() == 0) {
            srand(time(NULL) ^ getpid());
            while (1) {
                char task[TASK_SIZE];
                for (int j = 0; j < TASK_SIZE; j++) {
                    task[j] = 'a' + rand() % 26;
                }

                sem_wait(sem_empty); // Czekaj na wolne miejsce w kolejce
                sem_wait(sem_queue); // Zablokuj dostęp do kolejki

                // Dodaj zadanie do kolejki
                memcpy(shm->queue[shm->tail], task, TASK_SIZE);
                shm->tail = (shm->tail + 1) % QUEUE_SIZE;
                shm->count++;

                sem_post(sem_queue); // Odblokuj dostęp do kolejki
                sem_post(sem_full); // Zwiększ liczbę zadań w kolejce

                printf("Użytkownik %d dodal zadanie: %.*s\n", getpid(), TASK_SIZE, task);
                sleep(rand() % 5 + 1); // Odczekaj losową liczbę sekund
            }
            exit(0);
        }
    }
    for (int i = 0; i < M; i++) {
        if (fork() == 0) {
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
            exit(0);
        }
    }

    // Czekaj na zakończenie procesów (nigdy się nie kończy w tym przypadku)
    while (wait(NULL) > 0);

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