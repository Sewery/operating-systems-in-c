#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>
#include <sys/mman.h>
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
int main(int argc, char *argv[]) {
     if (argc != 3) {
        fprintf(stderr, "Użycie: %s szerokość prostokąta, maks. liczba procesów>\n", argv[0]);
        return 1;
    }

    double width = atof(argv[1]);
    int max_k = atoi(argv[2]);
    for(int k=1;k<=max_k;k++){
        struct timespec start_time, end_time;
        clock_gettime(CLOCK_REALTIME, &start_time);
        double result=0.0;
        double part=1.0/k;
        int fd[k][2], w;
        for(int i=0;i<k;i++){
            double d;
            pipe(fd[i]);
            pid_t p = fork();
            if(p==0){
                close(fd[i][0]); 
                double part_res = integrate(part*i,part*(i+1),width);
                w = write(fd[i][1], &part_res, sizeof(double)); 
                close(fd[i][1]); 
                exit(0);
            }
            close(fd[i][1]);           
        }
        for(int i=0;i<k;i++){
            double part_res=0.0;
            read(fd[i][0],&part_res,sizeof(double));
            close(fd[i][0]);
            result+=part_res;
        }
        clock_gettime(CLOCK_REALTIME, &end_time);

        long seconds = end_time.tv_sec - start_time.tv_sec;
        long nanoseconds = end_time.tv_nsec - start_time.tv_nsec;
        if (nanoseconds < 0) {
            seconds--;
            nanoseconds += 1e9;
        }
        printf("Dla k=%d wynik to: %f, czas wykoniania: %ld sekund i %ld nanosekund\n",k,result,seconds, nanoseconds);
    }

}