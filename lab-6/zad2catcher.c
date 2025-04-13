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
// Funkcja do całkowania
double f(double x) {
    return 4.0 / (x * x + 1.0);
}

// Część całkowania obliczana przez dziecko
double integrate(double from, double to, double width) {
    double sum = 0.0;
    for (double x = from + width / 2.0; x < to; x += width) {
        sum += f(x) * width;
    }
    return sum;
}
struct pak{
    double from;
    double to;
};
int main(int argc, char * argv[]){
    int w1;
    struct pak ob;
    //Odebranie przedziału
    w1 = open("potok1",O_RDONLY);
    read(w1,&ob,sizeof(ob));
    close(w1);
    printf("Odebranie przedziału od %f  do %f\n",ob.from,ob.to);
    //Wysyłanie wyniku
    double wynik =integrate(ob.from,ob.to,0.001);
    printf("Obliczanie i wysyłanie wyniku całkowania\n");
    mkfifo("potok2",S_IFIFO | S_IRWXU);
    w1= open("potok2",O_WRONLY);
    write(w1,&wynik,sizeof(double));
    close(w1);
    return 0;
}   