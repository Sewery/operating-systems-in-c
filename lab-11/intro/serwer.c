#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
int main(void){
    int status, gniazdo, dlugosc, nr=0, end=1, gniazdo2;
    struct sockaddr_in ser, cli;
    char buf[200];

    gniazdo = socket(AF_INET, SOCK_STREAM, 0);
    if(gniazdo == -1){
        printf("blad socket\n");
        return 1;  // Lepiej zwrócić 1 jako kod błędu
    }

    memset(&ser, 0, sizeof(ser));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(9000);
    ser.sin_addr.s_addr = htonl(INADDR_ANY); // Nasłuchuje na wszystkich interfejsach
    status = bind(gniazdo, (struct sockaddr*)&ser, sizeof ser);
    if(status == -1){
        printf("blad bind\n");
        close(gniazdo);  // Zamknięcie gniazda przed zakończeniem
        return 1;
    }

    status = listen(gniazdo, 10);
    if(status == -1){
        printf("blad listen\n");
        close(gniazdo);
        return 1;
    }

    printf("Serwer uruchomiony na porcie 9000. Oczekiwanie na połączenia...\n");

    while(end){
        dlugosc = sizeof cli;
        gniazdo2 = accept(gniazdo, (struct sockaddr*)&cli, (socklen_t *)&dlugosc);
        if(gniazdo2 == -1){
            printf("blad accept\n");
            continue;  // Kontynuuj pętlę zamiast kończyć program
        }

        read(gniazdo2,buf,sizeof buf);
        
        // Poprawiona logika warunków
        if(buf[0] == 'Q'){
            sprintf(buf, "ZGODA,SERWER KONCZY PRACE");
            end = 0;
        } else {
            sprintf(buf, "Jestes klientem nr %d", nr++);
        }

        write(gniazdo2,buf,strlen(buf));
        close(gniazdo2); 
    }

    close(gniazdo);
    printf("KONIEC serwera\n");
    return 0;
}