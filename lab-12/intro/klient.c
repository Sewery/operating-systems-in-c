int main(void){
    int status, gniazdo, dlugosc,nr=0,end=0,gniazdo2,l_bajtow;
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
    ser.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr
    //("149.156.207.21");// inet_addr("127.0.0.1");

    status = bind(gniazdo, (struct sockaddr*) &ser,sizeof ser);
    if(status==-1){
        printf("blad bind\n");
        return -1;
    }

    while(1){
        dlugosc = sizeof cli;
        l_bajtow = recvfrom(gniazdo, buf, sizeof(buf),0,(struct sockaddr*)&cli, (socklen_t *) &dlugosc);
        buf[l_bajtow]=0;
        printf("Otrzymano: %s\n",buf);
        fflush(stdout);
        if(l_bajtow > 0){
            l_bajtow = sendto(gniazdo,"to jest odpowiedz od serwera",28,0,(struct sockaddr*)&cli,dlugosc);
        }
    }
    close(gniazdo);

    return 0;
}