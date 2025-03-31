#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>

void obsulga_v1(int signum, siginfo_t * si,void * p3){
    printf("Obsluga sygnalu v1: %d %d, war%d\n",si->si_pid,si->si_uid,si->si_value);
}
int main(void){
    sigset_t set;
    struct sigaction act;
    printf("pid:%d\n",getpid());

    act.sa_sigaction = obsulga_v1;
    sigemptyset(&act,sa_mask);
    act.sa_flags= SA_SIGINFO;
    sigaction(SIGUSR1,&act,NULL);
}