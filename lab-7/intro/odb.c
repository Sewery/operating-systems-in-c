#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
struct pakiet{
    long typ;
    int zawartosc;
};
int main(void){
    struct pakiet o1;
    key_t klucz = ftok("./plik2",'p');
    int id_kolejki_kom = msgget(klucz,0600);

    //odbior komunikatow typu 1
    while(1){
        if(msgrcv(id_kolejki_kom,&o1,sizeof(struct pakiet),1,IPC_NOWAIT)<0)break;
        printf("otrzymano: %d\n",o1.zawartosc);
    }
    //odbior komunikatow typu 2
    while(1){
        if(msgrcv(id_kolejki_kom,&o1,sizeof(struct pakiet),2,IPC_NOWAIT)<0)break;
        printf("otrzymano: %d\n",o1.zawartosc);
    }
    // msgctl(id_kolejki_kom, IPC_RMID,NULL);//usuniecie kolejki
    return 0;
}