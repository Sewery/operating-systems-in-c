#include <stdio.h>
#include "bibl1.h"
int collatz_conjecture(int input){
	return input%2==0?input/2:3*input+1;
}
int test_collatz_convergence(int input, int max_iter,int *steps){
	if(input==1){
		return 1;
	}
	int counter_steps=0;
	while(input!=1 && counter_steps<max_iter){
		input=collatz_conjecture(input);
		steps[counter_steps]=input;
		counter_steps++;
	}
	if(input!=1){
		return 0;
	}
	return counter_steps;
}
