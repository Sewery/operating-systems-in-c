#include <sys/msg.h> 
#include <sys/ipc.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define INIT 1
#define CONNECTED 2

struct pakiet{ 
    long typ;
    int id; // albo klucz albo nadane id przez serwera dla klienta
    char tresc[100];
};

struct klient{
    int id;
    int msq_id;
};
static int last_id=0;
static struct klient klienci[100];

int main(void){
    //Informacje o klientach w tablicy
    //0. Tworzenie nowej kolejki komunikatow
    key_t klucz = ftok("./plik",'p');
    int id_kolejki_serwera = msgget(klucz,0600 | IPC_CREAT);
    if (id_kolejki_serwera == -1) {
        perror("msgget error (server)");
        exit(1);
    }
    printf("Id kolejki serwera: %d\n",id_kolejki_serwera);
    struct pakiet o2;
    while(1){
        //1.
        //a) Serwer po otrzymaniu komunikatu INIT, otwiera kolejke klienta i przydzielenie mu id
        //b) Odeslanie id klienta do niego, poprzez kolejke klienta
        if(msgrcv(id_kolejki_serwera,&o2,sizeof(struct pakiet)-sizeof(long),INIT,IPC_NOWAIT)>=0){
            if (o2.id == -1) {
                perror("msgget error (client queue)");
                continue;
            }
            klienci[last_id].id=last_id;
            klienci[last_id].msq_id=o2.id;
            o2.id=last_id;
            o2.typ=CONNECTED;
            last_id++;
            if (msgsnd(klienci[last_id-1].msq_id,&o2,sizeof(struct pakiet)-sizeof(long),IPC_NOWAIT)){
                perror("msgsnd error (send client ID)");
            }
            printf("Nowy klient z id %d zostal polaczony\n",last_id-1); 
        }
        //2.
        //a) Oczekiwanie na komunikaty od klienta
        //b) Przesyłanie je do innych klientow
        if(msgrcv(id_kolejki_serwera,&o2,sizeof(struct pakiet)-sizeof(long),CONNECTED,IPC_NOWAIT)>=0){
            o2.typ=CONNECTED;
            for(int i=0;i<last_id;i++){
                msgsnd(klienci[i].msq_id,&o2,sizeof(struct pakiet)-sizeof(long),IPC_NOWAIT);
            }
            printf("%d",last_id);
            printf("Wiadomosc została zbroadcastowana do klientow\n");
        }
    }           
         

    msgctl(id_kolejki_serwera, IPC_RMID,NULL);
}