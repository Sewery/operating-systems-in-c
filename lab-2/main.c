#include "bibl1.h"
#include <stdio.h>
#define MAX_ITER 20
int main() {
    int tab[MAX_ITER]={};
    for(int i=1;i<=30;i++){
        int steps=test_collatz_convergence(i,MAX_ITER,tab);
        printf("Number of steps for %d: %d\n",i,steps);
        if(steps==0){
            printf("Fail. Collatz algorithm did not complete %d iterations\n",MAX_ITER);
            continue;
        }
        printf("Steps:\n");
        for(int i=0;i<steps;i++){
            printf("%d\n",tab[i]);
        }
        printf("##############\n");
    }
    
}