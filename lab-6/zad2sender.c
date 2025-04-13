#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h> 
#include <sys/stat.h>
struct pak{
    double from;
    double to;
};
int main(int argc, char * argv[]){
    if (argc != 3) {
        fprintf(stderr, "Złe dane wejściowe \n");
        return 1;
    }
    int w1;
    double from = atof(argv[1]);
    double to = atof(argv[2]);
    struct pak ob;
    ob.from=from;
    ob.to=to;
    printf("Wysłanie przedziału od %f  do %f\n",from,to);
    mkfifo("potok1",S_IFIFO | S_IRWXU);
    w1= open("potok1",O_WRONLY);
    write(w1,&ob,sizeof(ob));
    close(w1);
    //Odbieranie wyniku
    double wynik=0.0;
    w1= open("potok2",O_RDONLY);
    read(w1,&wynik,sizeof(wynik));
    printf("Otrzymany wynik całkowania: %f \n",wynik);
    close(w1);
    return 0;
}   