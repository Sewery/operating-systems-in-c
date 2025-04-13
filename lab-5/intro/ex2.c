#include <stdio.h>
#include <signal.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <bits/sigaction.h>
#include <bits/types/sigset_t.h>



int main(void){
    sigset_t newmask, oldmask, set;
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);

    raise(SIGUSR1);
    sigpending(&set);
    if (sigismember(&set, SIGUSR1) == 1){
        printf("SIGUSR1 oczekuje na odblokowanie(1)\n");
    }
}