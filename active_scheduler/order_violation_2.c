#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#include "trigger.h"
#include "instrumenter.h"

pthread_mutex_t lock;

static int a = 0;
static int b = 0;
static int c = 0;

static int pending;

void *func1(void *arg){

	inst_begin(1001, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 1;
	inst_end(1001);

	inst_begin(1002, VAR, &pending, MODE_WRITE, NULL, 0, 0);
	pending = 1;
	inst_end(1002);

	inst_begin(1003, VAR, &pending, MODE_READ, NULL, 0, 0);
	while(pending){
		c = 1;
		pending = 0;
	}
	inst_end(1003);

	inst_begin(1004, VAR, &pending, MODE_WRITE, NULL, 0, 0);
	b = 1;
	inst_end(1004);

	// If value of c is 0, bug manifests. 
	printf("The value of c is %d\n", c);

	// Return value of c to its initial value. 
	c = 0;

	return NULL;
}

void *func2(void *arg){

	inst_begin(2001, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 2;
	inst_end(2001);

	inst_begin(2002, VAR, &pending, MODE_WRITE, NULL, 0, 0);
	pending = 0;
	inst_end(2002);

	inst_begin(2003, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 2;
	inst_end(2003);

	return NULL;
}

int main(int argc, char const *argv[])
{
	pthread_mutex_init(&lock, NULL);
	activate_profiler();
	pthread_t t1;
	pthread_t t2;
	pthread_create(&t1, NULL, func1, NULL);
	pthread_create(&t2, NULL, func2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	create_exec_order(1);
	activate_scheduler();
	test(t1, t2, func1, func2);
	pthread_mutex_destroy(&lock);

	return 0;
}
