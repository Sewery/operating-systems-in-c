#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int main(void){
    pid_t pid;
    int i;
    for(i=0;i<10;i++){
        printf("Proces główny %p %d\n",(void*)&i,i);
        pid=fork();
        if(pid==0){
            printf("Proces potomny %p %d\n",(void*)&i,i);
            return 0;
        }

    }
    while(wait(0) >0);

    return 0;
}