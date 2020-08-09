#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "trigger.h"
#include "instrumenter.h"

pthread_mutex_t lock;

int a = 0;
int b = 0;
int c = 0;
int d = 0;

void *
func1(void *arg)
{
	inst_begin(1001, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 1;
	inst_end(1001);

	inst_begin(1002, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 2;
	inst_end(1002);

	inst_begin(1003, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 2;
	inst_end(1003);

	inst_begin(1004, VAR, &b, MODE_READ, NULL, 0, 0);
	if (b == 1)
	{
		crash();
	}
	inst_end(1004);

	inst_begin(1005, VAR, &c, MODE_WRITE, NULL, 0, 0);
	c = 1;
	inst_end(1005);

	// inst_begin(1006, SYNC, (int *)&lock, MODE_READ, &lock, 0, 0);
	// pthread_mutex_lock(&lock);
	// inst_end(1006);

	// inst_begin(1007, VAR, &d, MODE_WRITE, &lock, 1, 0);
	// d = 1;
	// inst_end(1007);

	// inst_begin(1008, VAR, &d, MODE_WRITE, &lock, 0, 1);
	// d = 2;
	// inst_end(1008);

	// inst_begin(1009, SYNC, (int *)&lock, MODE_READ, &lock, 0, 0);
	// pthread_mutex_unlock(&lock);
	// inst_end(1009);

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
	inst_begin(2001, VAR, &a, MODE_READ, NULL, 0, 0);
	if(a == 1)
	{
		crash();
	}
	inst_end(2001);

	inst_begin(2002, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 1;
	inst_end(2002);

	inst_begin(2003, VAR, &c, MODE_WRITE, NULL, 0, 0);
	c = 2;
	inst_end(2003);

	inst_begin(2004, VAR, &c, MODE_READ, NULL, 0, 0);
	if (c == 1)
	{
		crash();
	}
	inst_end(2004);

	// inst_begin(2005, SYNC, (int *)&lock, MODE_READ, &lock, 0, 0);
	// pthread_mutex_lock(&lock);
	// inst_end(2005);

	// inst_begin(2006, VAR, &d, MODE_READ, &lock, 1, 1);
	// if (d == 1)
	// {
	// 	crash();
	// }
	// inst_end(2006);

	// inst_begin(2007, SYNC, (int *)&lock, MODE_READ, &lock, 0, 0);
	// pthread_mutex_unlock(&lock);
	// inst_end(2007);

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

	exit(0);
}
