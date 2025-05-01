#include <sys/msg.h> 
#include <sys/ipc.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define INIT 1
#define CONNECTED 2

struct pakiet{ 
    long typ;
    int id; 
    char tresc[100];
};
char inp [100];
static int id=-1;
int main(int argc,char *argv[]){
    if (argc != 2) {
        fprintf(stderr, "Prosze uzyc jako argumentu id kolejki serwera\n");
        exit(1);
    }
    int id_kolejki_serwera =atoi(argv[1]);
    // 1.
    //a) Tworzenie kolejki z unikalnym kluczem IPC,
    key_t klucz = ftok("./plik2",getpid());
    if(klucz==-1){
        perror("ftok (client private) error");
        return 1;
    }
    int id_kolejki_klienta = msgget(klucz,0600 | IPC_CREAT | IPC_EXCL);
    if (id_kolejki_klienta == -1) {
        perror("msgget (client private) error");
        exit(1);
    }
    //b) Wysłanie klucza do serwera wraz z komunikatem INIT
    struct pakiet o1;
    o1.typ = INIT;
    o1.id = id_kolejki_klienta;
    if (msgsnd(id_kolejki_serwera,&o1,sizeof(struct pakiet)-sizeof(long),0) == -1){
        perror("msgsnd (INIT to server) error");
        exit(1);
    }
    struct pakiet o2;
    if(msgrcv(id_kolejki_klienta,&o2,sizeof(struct pakiet)-sizeof(long),CONNECTED,0)<0){
        perror("Can't receive id form server ");
        exit(1);
    }
    printf("Nawiązano połączenie z serwerem. Dostano id: %d\n",o2.id);
    id=o2.id;
    
    //2. utworzenie nowego procesu, który odbiera komunikaty, wysłane przez serwer, przy uzyciu
    pid_t p2= fork();
    if(p2==0){
        while(1){
            if(msgrcv(id_kolejki_klienta,&o2,sizeof(struct pakiet)-sizeof(long),CONNECTED,0)>=0){
                printf("%d: %s\n",o2.id, o2.tresc);
            }
        }
    }
    //3. sczytywanie z wejscia komunikatow do wyslania, po nadaniu id klientowi
    else{
        while(1){
            scanf("%s",inp);
            if(id<0){
                perror("Klient nie ma jeszcze przydzielonego id");
                continue;
            }
            o2.typ = CONNECTED; 
            o2.id=id;
            strcpy(o2.tresc,inp);
            if(msgsnd(id_kolejki_serwera,&o2,sizeof(struct pakiet)-sizeof(long),0)){
                perror("Cos poszlo nie tak");
                exit(0);
            }
        }
    }
    msgctl(id_kolejki_klienta, IPC_RMID,NULL);

}