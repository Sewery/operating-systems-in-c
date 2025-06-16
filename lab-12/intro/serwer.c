#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int main (void){
    int status, gniazdo, dlugosc, nr=0, end=1, gniazdo2, l_bajtow;
    struct sockaddr_in ser,cli;
    char buf[200];
    gniazdo = socket(AF_INET,SOCK_DGRAM,0);
    if(gniazdo == -1){
        printf("blad socket \n");
        return -1;
    }

    memset(&ser,0,sizeof(ser));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(9001);
    ser.sin_addr.s_addr = inet_addr("149.156.207.21");//inet_addr
    ("127.0.0.1");//htonl(INADOOR_ANY)
    //if(inet_pton(AF_INET,"149.156.207.21", &(ser.sin_addr.s_addr))!=1) return -1
    l_bajtow = sendto(gniazdo, "to jest odpowiedz od serwera",28,0,(struct sockaddr*)&cli, (socklen_t *)&dlugosc);
    buf[l_bajtow]=0;
    printf("Otrzymano: %s\n",buf); fflush(stdout);

    

}
          