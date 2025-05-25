#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>
#include <sys/mman.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

double width;
// Funkcja do całkowania
double f(double x) {
    return 4.0 / (x * x + 1.0);
}

// Część całkowania obliczana przez dziecko
double integrate(double from, double to) {
    double sum = 0.0;
    for (double x = from + width / 2.0; x < to; x += width) {
        sum += f(x) * width;
    }
    return sum;
}
// Tablica gotowosci wykorzystac 
double wynik[20];
struct pakiet{
    int id_watka;
    double from;
    double to;
};
void * fun_watka(void * val){
    struct pakiet * pak = (struct pakiet*)val;
    double part_res = integrate(pak->from,pak->to);
    wynik[pak->id_watka]=part_res;
    // sleep(1);
    free(pak);
}
int main(int argc, char *argv[]) {
     if (argc != 3) {
        fprintf(stderr, "Prosze uzyc jako argumentow szerokość prostokąta i maks. liczba watków\n");
        return 1;
    }

    width = atof(argv[1]);
    int max_k = atoi(argv[2]);
    for(int k=1;k<=max_k;k++){
        struct timespec start_time, end_time;
        clock_gettime(CLOCK_REALTIME, &start_time);
        double result=0.0;
        double part=1.0/k;
        pthread_t watki[k];
        for(int i=0;i<k;i++){
            struct pakiet * ob = malloc(sizeof(struct pakiet));
            ob->id_watka=i;
            ob->from =part*i;
            ob->to =part*(i+1);
            (void) pthread_create(&watki[i], NULL, &fun_watka, (void*)ob);    
        }
        for(int i=0;i<k;i++){
            pthread_join(watki[i],NULL);
        }
        for(int i=0;i<k;i++){
            result+=wynik[i];
        }
        clock_gettime(CLOCK_REALTIME, &end_time);

        long seconds = end_time.tv_sec - start_time.tv_sec;
        long nanoseconds = end_time.tv_nsec - start_time.tv_nsec;
        if (nanoseconds < 0) {
            seconds--;
            nanoseconds += 1e9;
        }
        printf("Dla k=%d wynik to: %f, czas wykoniania: %ld sekund i %ld nanosekund\n",k,result,seconds, nanoseconds);
    }

}