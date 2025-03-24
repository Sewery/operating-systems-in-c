#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    pid_t pid;
    int max = *argv[1]-'0';
    pid_t pid_main= getpid();
    for(int i=0;i<max;i++){
        pid=fork();
        if(pid==0){
            printf("PID procesu macierzystego %d, PID procesu potomengo %d\n",getppid(),getpid());
            break;
        }

    }
    while(wait(0) >0);
    if(getpid()==pid_main){
        printf("Koniec %s\n",argv[1]);
    }
    return 0;
}