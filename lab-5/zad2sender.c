#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void signal_handler(int no_sig){
    printf("Confirmed catching signal %d\n",no_sig);
}
int main(int argc, char *argv[]) {
    if(argc!=3){
        fprintf(stderr,"Error invalid number of arguments");
        return EXIT_FAILURE;
    }

    //Iniclizacja odbioru sygnalu zwrotnego
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1); 
    sigprocmask(SIG_BLOCK, &mask, &oldmask); 

    struct sigaction act_catcher;
    act_catcher.sa_handler=signal_handler;
    sigemptyset(&act_catcher.sa_mask);
    act_catcher.sa_flags = 0;


    int senderPid = atoi(argv[1]);
    int mode = atoi(argv[2]);
    //Wyslanie sygnalu do catchera

    union sigval war;
    war.sival_int = mode;
    if (sigqueue(senderPid, SIGUSR1, war) == -1) {
         fprintf(stderr,"Error during sending message\n");
        return 1;
    }
    sigaction(SIGUSR1, &act_catcher, NULL);     
    //Czekanie na sygnal od catcher
    sigsuspend(&oldmask);
    return 0;
}