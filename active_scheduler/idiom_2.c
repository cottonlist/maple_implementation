#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "trigger.h"
#include "instrumenter.h"

pthread_mutex_t lock;

int a = 0;
int b = 0;

void *
func1 (void *arg)
{
	inst_begin(1001, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 1;
	inst_end(1001);

	inst_begin(1002, VAR, &a, MODE_READ, NULL, 0, 0);
	if (a == 2)
	{
		crash();
	}
	inst_end(1002);

	inst_begin(1003, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = b + 1;
	inst_end(1003);

	return NULL;
}

void *
func2 (void *arg)
{
	inst_begin(2001, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = a + 1;
	inst_end(2001);

	inst_begin(2002, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 1;
	inst_end(2002);

	inst_begin(2003, VAR, &b, MODE_READ, NULL, 0, 0);
	if (b == 2)
	{
		crash();
	}
	inst_end(2003);

	return NULL;
}

int
main()
{
	pthread_mutex_init(&lock, NULL);
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

	pthread_mutex_destroy(&lock);

	exit(0);
}
