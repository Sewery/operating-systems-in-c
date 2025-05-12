#include <stdio.h>
#include <time.h>
#include <pthread.h>

pthread_t watek[10];
int wynik[10];

void* fun_watka(void* cos){
    int i = *(int*)cos;
    printf("Watek %d\n", i);
    fflush(stdout);
    sleep(i);
    char *zm;
    zm=malloc(sizeof(char));
    *zm='A'+i;
    return (void*)&zm;
}

int main(void){
    int arg[10];
    for(int i=0; i<10; i++){
        wynik[i] = i;
        pthread_create(&watek[i], NULL, &fun_watka, &arg[i]);
    }
    char *wsk;
    for(int i=0;i<10;i++){
        pthread_join(watek[i],&wsk);
        printf("%c\n",*wsk);
        free(wsk);
    }
    return 0;
}
// kompilacja -lpthread