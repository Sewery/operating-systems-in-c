#include <dlfcn.h>
#include <stdio.h>
#define MAX_ITER 1000
int main(void){
	void *uchwyt =dlopen("./libbibl1.so",RTLD_LAZY);
	if(!uchwyt){
		printf("blad otw.biblioteki\n");
		return 0;
	}
	int (*f2)(int,int,int *);
	f2 = (int (*)(int,int,int *))dlsym(uchwyt,"test_collatz_convergence");
	if(dlerror()!=0){
		printf("blad fun 2\n");
		return 0;
	}
	int tab[MAX_ITER]={};
    for(int i=1;i<=30;i++){
        int steps=f2(i,MAX_ITER,tab);
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
	dlclose(uchwyt);
	return 0;
}
