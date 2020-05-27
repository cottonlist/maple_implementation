#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "trigger.h"

struct info infos[4];
int i = 0;

int a = 0;

//int exec_order[] = {2002, 2003, 2004, 1002, 1003, 2005};
int exec_order[] = {2001, 1001, 2002, 1002};
int exec_length = sizeof(exec_order) / sizeof(exec_order[0]);

void *
func1(void *arg)
{
	infos[i++] = inst_begin(1001, &a, 0);
	a = 1;
	inst_end(1001);

	infos[i++] = inst_begin(1002, &a, 0);
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
	infos[i++] = inst_begin(2001, &a, 1);
	assert(a != 1);
	inst_end(2001);

	infos[i++] = inst_begin(2002, &a, 0);
	a = 3;
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

	// int exec_order[] = {1001, 2001, 1002};
	
	inst_initialize(exec_order, exec_length);
	
	pthread_t t1;
	pthread_t t2;
	
	pthread_create(&t1, NULL, func1, NULL);
	pthread_create(&t2, NULL, func2, NULL);
	
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	inst_uninitialize();

	// for (int i = 0; i < 4; ++i)
	// {
	// 	printf("%p\n", infos[i].accessed_mem_addr);
	// }

	exit(0);
}
