#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
int main(){
    sem_t *id_sem = sem_open("nazwa_sem", O_CREAT, 0660);
    pid_t pid1 = fork();
    if(pid1==0){
        sleep(1);
        printf("czekamy potomny\n");fflush(stdout);
        sem_wait(id_sem);
        printf("wewnatrz potomny\n");fflush(stdout);
        sleep(3);
        sem_post(id_sem);
        printf("po potomny\n"); fflush(stdout);
        return 0;
    }else{
        sleep(2);
        printf("czekam glowny\n");fflush(stdout);
        sem_wait(id_sem);
        printf("wewnatrz glowny\n"); fflush(stdout);
        printf("po glowny\n"); fflush(stdout);
    }
    return 0;
}