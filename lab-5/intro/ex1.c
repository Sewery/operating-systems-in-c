#include <stdio.h>
#include <signal.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
void handler(int signum){
    printf("Obsługa sygnału\n");
}

int main(void){
    printf("%d\n",getpid());
    signal(SIGUSR1, handler);
    raise(SIGUSR1);
    while(1);
    return 0;
}