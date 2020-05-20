#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "trigger.h"

int a = 0;

int exec_order[] = {2002, 2003, 2004, 1002, 1003, 2005};
int exec_length = sizeof(exec_order) / sizeof(exec_order[0]);

void *
func1(void *arg)
{
	inst_begin(1001);
	a = 1;
	inst_end(1001);

	inst_begin(1002);
	a = 1;
	inst_end(1002);

	inst_begin(1003);
	a = 1;
	inst_end(1003);

	inst_begin(1004);
	a = 1;
	inst_end(1004);

	inst_begin(1005);
	a = 1;
	inst_end(1005);

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
	inst_begin(2001);
	a = 1;
	inst_end(2001);

	inst_begin(2002);
	a = 1;
	inst_end(2002);

	inst_begin(2003);
	a = 1;
	inst_end(2003);

	inst_begin(2004);
	a = 1;
	inst_end(2004);

	inst_begin(2005);
	a = 1;
	inst_end(2005);
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

	// int exec_order[] = {1001, 2001, 1002};
	
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
