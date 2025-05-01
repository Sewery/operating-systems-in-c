#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
struct pakiet{ 
    long typ; // pierwsze pole musi byc long  i stanowi ty
    int zawartosc; // dowolna zawartosc; rozmiar struktury <=  MSGMAX(4096)
    //char zaw_inna[100]
}o1;
int main(void){
    int i;
    key_t klucz = ftok("./plik1",'p'); //plik, jednoliterowy identyfikator projektu
    int id_kolejki_kom = msgget(klucz,IPC_CREAT | 0600);
    for(i=0;i<5;i++){
        o1.typ = (i%2) +1; //type >0
        o1.zawartosc = i;
        msgsnd(id_kolejki_kom,&o1,sizeof(struct pakiet),0);
    }
    return 0;

}
//ipcs -q