#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

//liczba pacjentów
int N;
//liczba farmaceutów
int M;

pthread_mutex_t mutex_farm_czeka = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_oczek_pacjenci = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_leki = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lekarz = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_konsultacja = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_leki =PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_lekarz =PTHREAD_COND_INITIALIZER;

static int oczekujacy_pacjenci=0;
static int farmaceuta_czeka=0;
static int leki=0;

int id_pacjentow_w_kolejce[3];

pthread_t pacjenci[1000];
pthread_t farmaceuci[100];
pthread_t lekarz;

void print_time() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
}

void * fun_pacjent(void* id){
    int id_pacjenta=*((int*)id);
    free(id);
    int czas_podrozy_do_szpitala=rand() % 4 + 2;
    print_time();
    printf("- Pacjent(%d): Ide do szpitala, bede za %d s\n",id_pacjenta,czas_podrozy_do_szpitala);
    sleep(czas_podrozy_do_szpitala);
        
    pthread_mutex_lock (&mutex_oczek_pacjenci);
    while(oczekujacy_pacjenci>=3){
        czas_podrozy_do_szpitala=rand() % 3 + 2;
        print_time();
        printf("- Pacjent(%d): za dużo pacjentów, wracam później za %d s\n",id_pacjenta,czas_podrozy_do_szpitala);
        pthread_mutex_unlock(&mutex_oczek_pacjenci); // Zwolnij mutex
        sleep(czas_podrozy_do_szpitala);
        pthread_mutex_lock(&mutex_oczek_pacjenci); 
    }
    // Pacjent siada w poczekalni
    id_pacjentow_w_kolejce[oczekujacy_pacjenci]=id_pacjenta;
    int id_w_kolejce=oczekujacy_pacjenci;
    oczekujacy_pacjenci+=1;//w sekcji
    print_time();
    printf("- Pacjent(%d): czeka %d pacjentów na lekarza\n",id_pacjenta,oczekujacy_pacjenci);
    if(oczekujacy_pacjenci==3){
        print_time();
        printf("- Pacjent(%d): budzę lekarza.\n",id_pacjenta);
        pthread_cond_signal(&cond_lekarz);
    }
    pthread_mutex_unlock(&mutex_oczek_pacjenci);


    pthread_mutex_lock (&mutex_oczek_pacjenci);
    while(id_pacjentow_w_kolejce[id_w_kolejce]==id_pacjenta)
        pthread_cond_wait(&cond_konsultacja, &mutex_oczek_pacjenci);
    pthread_mutex_unlock (&mutex_oczek_pacjenci);

    print_time();
    printf("- Pacjent(%d): kończę wizytę\n",id_pacjenta);
}

void* fun_farmaceuta(void* id){
    int id_farmaceuty=*((int*)id);
    free(id);

    int czas_podrozy_do_szpitala=rand() % 11 + 5;
    print_time();
    printf("- Farmaceuta(%d): Ide do szpitala, bede za %d s\n",id_farmaceuty,czas_podrozy_do_szpitala);
    sleep(czas_podrozy_do_szpitala);

    pthread_mutex_lock (&mutex_leki);
    while(leki>=3){
        print_time();
        printf("- Farmaceuta(%d): czekam na oproznienie apteczki\n",id_farmaceuty);
        pthread_cond_wait(&cond_leki, &mutex_leki);
    }
    pthread_mutex_unlock (&mutex_leki);

    pthread_mutex_lock (&mutex_farm_czeka);
    print_time();
    printf("- Farmaceuta(%d): budzę lekarza\n",id_farmaceuty);
    farmaceuta_czeka=1;
    pthread_cond_signal(&cond_lekarz);
    pthread_mutex_unlock (&mutex_farm_czeka);

    print_time();
    printf("- Farmaceuta(%d): dostarczam leki\n",id_farmaceuty);

    pthread_mutex_lock (&mutex_leki);
    while(leki!=6){
        pthread_cond_wait(&cond_leki, &mutex_leki);
    }
    pthread_mutex_unlock (&mutex_leki);
    print_time();
    printf("- Farmaceuta(%d): zakończyłem dostawę\n",id_farmaceuty);
}

void* fun_lekarza(void* arg){
    while(1){
        pthread_mutex_lock (&mutex_lekarz);
        while(!(oczekujacy_pacjenci==3 && leki>=3)  && !(farmaceuta_czeka==1 && leki<3)){
            pthread_cond_wait(&cond_lekarz, &mutex_lekarz);
        }
        pthread_mutex_unlock (&mutex_lekarz);
        print_time();
        printf("- Lekarz: budzę się\n");

        if(oczekujacy_pacjenci==3 && leki>=3){
            pthread_mutex_lock (&mutex_oczek_pacjenci);
            pthread_mutex_lock (&mutex_leki);

            print_time();
            printf("- Lekarz: konsultuję pacjentów (%d, %d, %d)\n",id_pacjentow_w_kolejce[0],id_pacjentow_w_kolejce[1],id_pacjentow_w_kolejce[2]);

            leki-=3;
            oczekujacy_pacjenci=0;
            id_pacjentow_w_kolejce[0]=0;
            id_pacjentow_w_kolejce[1]=0;
            id_pacjentow_w_kolejce[2]=0;

            sleep(2+rand() % 3); // czas konsultacji
            pthread_mutex_unlock (&mutex_leki);
            pthread_mutex_unlock (&mutex_oczek_pacjenci);
           
            pthread_cond_broadcast(&cond_konsultacja);
        }else if(leki<3 && farmaceuta_czeka==1){
            pthread_mutex_lock (&mutex_farm_czeka);
            pthread_mutex_lock (&mutex_leki);
            
            leki=6;
            farmaceuta_czeka=0;

            print_time();
            printf("- Lekarz: przyjmuję dostawę leków\n");

            sleep(rand() % 3+1);
            pthread_mutex_unlock (&mutex_leki);
            pthread_mutex_unlock (&mutex_farm_czeka);

            pthread_cond_broadcast(&cond_leki);
        }
        print_time();
        printf("- Lekarz: zasypiam\n");
    }
}
int main(int argc, char *argv[]){
    printf("%d\n",argc);
    // if (argc != 3) {
    //     fprintf(stderr, "Prosze uzyc jako argumentow liczbe pacjentow i lekarzy\n");
    //     return 1;
    // }
    N = 6;
    M = 2;

    srand(time(NULL));

    for(int i=0;i<N;i++){
        int* id=malloc(sizeof(int));
        *id=i+1;
        (void) pthread_create(&pacjenci[i], NULL, &fun_pacjent, (void*)id);
    }

    for(int i=0;i<M;i++){
        int* id=malloc(sizeof(int));
        *id=i+1;
        (void) pthread_create(&farmaceuci[i], NULL, &fun_farmaceuta, (void*)id);
    }

    (void) pthread_create(&lekarz, NULL, &fun_lekarza, NULL);

    
    for (int i = 0; i < N; i++) {
        pthread_join(pacjenci[i], NULL);
    }
    for (int i = 0; i < M; i++) {
        pthread_join(farmaceuci[i], NULL);
    }

    pthread_cancel(lekarz);
}