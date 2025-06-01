#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int main(void){

    
    int status,gniazdo;
    struct sockaddr_in server,client;
    char buf[200];

    gniazdo = socket(AF_INET,SOCK_STREAM,0);
    if(gniazdo==-1){
        printf("blad socket\n");
        return 0;
    }

    memset(&server,0,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(9000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    status = connect(gniazdo, (struct sockaddr*)&server, sizeof server);
    if(status < 0){printf("blad connect\n"); return 0;}

    printf("Podaj tekst:");
    fgets(buf, sizeof(buf), stdin);
    status = write(gniazdo, buf, sizeof(buf));
    status = read(gniazdo, buf, sizeof(buf)-1);
    buf[status]='\0';
    printf("Otrzymalem: %s\n",buf);
    
    close(gniazdo);

}