#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "trigger.h"
#include "instrumenter.h"

pthread_mutex_t lock;

static int a;
static int b;

struct obj
{
	int state;
};

static struct obj mono;
static struct obj *obj1;

void *func1(void *arg){

	inst_begin(1001, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 1;
	inst_end(1001);

	inst_begin(1002, VAR, &obj1->state, MODE_WRITE, NULL, 0, 0);
	obj1->state = 20;
	inst_end(1002);

	inst_begin(1003, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 1;
	inst_end(1003);

	return NULL;
}

void *func2(void *arg){

	inst_begin(2001, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 2;
	inst_end(2001);

	// This read should be executed before instruction 1002. 
	inst_begin(2002, VAR, &obj1->state, MODE_READ, NULL, 0, 0);
	printf("%d\n", obj1->state);
	obj1->state = 0;
	inst_end(2002);
	// If output is 0, that means bug manifests. 

	inst_begin(2003, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 2;
	inst_end(2003);

	return NULL;
}

int main(int argc, char const *argv[])
{
	obj1 = &mono;

	pthread_mutex_init(&lock, NULL);
	// run profiler
	activate_profiler();
	pthread_t t1;
	pthread_t t2;
	pthread_create(&t1, NULL, func1, NULL);
	pthread_create(&t2, NULL, func2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	create_exec_order(1);
	activate_scheduler();

	// run active scheduler
	test(t1, t2, func1, func2);

	pthread_mutex_destroy(&lock);

	return 0;
}