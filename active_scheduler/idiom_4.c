#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "trigger.h"

int exec_order[] = {2001, 1001, 2002, 2003, 1003, 2004};
int exec_length = sizeof(exec_order)/sizeof(exec_order[0]);

int a = 0;
int b = 0;
int c = 0;

void *
func1(void *arg)
{
	inst_begin(1001);
	a = 1;
	inst_end(1001);
	inst_begin(1002);
	c = 1;
	inst_end(1002);
	inst_begin(1003);
	if (b == 1)
	{
		crash();
	}
	inst_end(1003);

	return NULL;
}

void *
func2(void *arg)
{
	inst_begin(2001);
	a = 3;
	inst_end(2001);
	inst_begin(2002);
	if (a == 1)
	{
		inst_end(2002);
		inst_begin(2003);
		b = 1;
		inst_end(2003);
	}
	inst_begin(2004);
	b = 2;
	inst_end(2004);

	return NULL;
}

int
main()
{
	inst_initialize(exec_order, exec_length);

	pthread_t t1;
	pthread_t t2;

	pthread_create(&t1, NULL, func1, NULL);
	pthread_create(&t2, NULL, func2, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	inst_uninitialize();

	exit(0);
}
