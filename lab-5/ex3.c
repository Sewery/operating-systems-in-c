#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char * tab[]){
    union sigval war;
    int pid=atoi(tab[1]);
    war.sival_int = 123;
    sigqueue(pid,SIGUSR1,war);
    return 0;
}
