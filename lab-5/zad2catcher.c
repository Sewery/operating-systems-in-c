#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int mode = 0;
int count = 0;
pid_t sender_pid = 0;
void signal_handler(int sig,siginfo_t *info, void *ucontext){
    mode = info->si_value.sival_int;
    sender_pid = info->si_pid;
    printf("Sygnał dostarczony w trybie %d\n",mode);
    kill(sender_pid, SIGUSR1); 
}
void sigint_signal_handler(int sig){
    printf("Wciśnięto CTRL+C , zmieniono obsługę SIGINT\n");
}
int main() {
    printf("PID catchera: %d\n", getpid());

     //Iniclizacja odbioru sygnalu zwrotnego
    struct sigaction act_catcher;
    act_catcher.sa_sigaction=signal_handler;
    sigemptyset(&act_catcher.sa_mask);
    act_catcher.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act_catcher, NULL);
    // int current_mode = 0;
    while(1){

        switch (mode) {
            case 0:
                break;
            case 1:
                printf("Liczba zmian trybów pracy %d\n",count);
                count++;
                mode=0;
                break;
            case 2:
                int i=0;
                while(mode==2){
                    printf("%d\n",i);
                    sleep(1);
                    i++;
                }
                 count++;
                mode=0;
                break;
            case 3:
                struct sigaction act_ignore;
                act_ignore.sa_handler=SIG_IGN;
                sigemptyset(&act_ignore.sa_mask);
                sigaddset(&act_ignore.sa_mask,SIGINT);
                sigaction(SIGINT, &act_ignore, NULL);
                act_ignore.sa_flags = 0;
                count++;
                 mode=0;
                break;
            case 4:
                struct sigaction act_write;
                act_write.sa_handler=sigint_signal_handler;
                sigemptyset(&act_write.sa_mask);
                sigaddset(&act_write.sa_mask,SIGINT);
                sigaction(SIGINT, &act_write, NULL);
                act_write.sa_flags = 0;
                count++;
                mode=0;
                break;
            case 5:
                return 0;
            default:
                fprintf(stderr, "Nieznany tryb: %d\n", mode);
        }
    }

    return 0;
}