#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>

int main(int argc, char *argv[]){
    pid_t pid;
    char * path = argv[1];
    int global=0;
    int local=0;
    pid = fork();
    if(pid<0){
        perror("Error when creating potomny process");
        return 0;
    }else if(pid!=0){
        //proces rodzica
        int status =0;
        wait(&status);
        printf("parent process\n");
        printf("parent pid = %d, child pid = %d\n",getpid(),pid);
        printf("Child exit code: %d\n",WEXITSTATUS(status));
        printf("Parent's local = %d, parent's global = %d , address=%p\n",local,global,(void*)&global);

        if(WEXITSTATUS(status)){
            printf("Child bad exit code status\n");
            return 1;
        }

    }else if(pid==0){
        // proces dziecka
        global++;
        local++;
        printf("child pid = %d, parent pid = %d\n",getpid(),getppid());
        printf("child's local = %d, child's global = %d, address=%p\n",local,global,(void*)&global);
        execl("/bin/ls","ls",argv[1],NULL);
    }
    return 0;
}