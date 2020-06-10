#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "trigger.h"
#include "instrumenter.h"

int a = 0;
int b = 0;

void *
func1(void *arg)
{
	inst_begin(1001, &a, MODE_WRITE);
	a = 1;
	inst_end(1001);

	inst_begin(1002, &a, MODE_WRITE);
	a = 2;
	inst_end(1002);

	// inst_begin(1001);
	// a = 1;
	// inst_end(1001);
	// inst_begin(1002);
	// a = 2;
	// inst_end(1002);
	return NULL;
}

void *
func2 (void *arg)
{
	inst_begin(2001, &a, MODE_READ);
	if(a == 1)
	{
		crash();
	}
	inst_end(2001);

	inst_begin(2002, &b, MODE_WRITE);
	b = 1;
	inst_end(2002);

	// inst_begin(2001);
	// if (a==1) {
	// 	crash();
	// }
	// inst_end(2001);
	return NULL;
}

int
main(void)
{	

	// run profiler
	activate_profiler();
	pthread_t t1;
	pthread_t t2;
	pthread_create(&t1, NULL, func1, NULL);
	pthread_create(&t2, NULL, func2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	create_exec_order();
	activate_scheduler();

	// run active scheduler
	test(t1, t2, func1, func2);

	exit(0);
}
