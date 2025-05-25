#include <stdio.h>
#include <time.h>
#include <pthread.h>

pthread_mutex_t mutex_oczek_pacjenci = PTHREAD_MUTEX_INITIALIZER;
pthread_t watek01, watek02;
int i;

void * fun_watka(void* cos){
    static int a=10;
    while(1){
        pthread_mutex_lock (&mutex_oczek_pacjenci);
        a=a+1;//w sekcji
        pthread_mutex_unlock (&mutex_oczek_pacjenci);
        printf("%s %d %d\n",(char*)cos,a,i);
        fflush(stdout);
        sleep(1);
    }
}
int main(void){
    pthread_create(&watek01, NULL, &fun_watka,"A");
    pthread_create(&watek02,NULL,&fun_watka,"B");

    sleep(4);
    return 0;

}