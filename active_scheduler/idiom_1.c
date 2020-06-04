#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "trigger.h"
#include "instrumenter.h"

// struct info thd1[2];
// struct info thd2[2];
// int thd1_index = 0;
// int thd2_index = 0;
// int exec_order[10][10];
// int exec_order_index = 0;

// int profiled_ins[4];

int a = 0;
int b = 0;

//int exec_order[] = {2002, 2003, 2004, 1002, 1003, 2005};
int exec_order[] = {2001, 1001, 2002, 1002};
int exec_length = sizeof(exec_order) / sizeof(exec_order[0]);

void *
func1(void *arg)
{
	inst_begin(1001, &a, 0);
	a = 1;
	inst_end(1001);

	inst_begin(1002, &a, 0);
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
	inst_begin(2001, &a, 1);
	assert(a != 1);
	inst_end(2001);

	inst_begin(2002, &b, 0);
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

	// int exec_order[] = {1001, 2001, 1002};
	// first run is to execute profiler
	// profiler_initialize();

	profiler_activator();
	test();
	
	// inst_initialize(exec_order, exec_length);
	
	pthread_t t1;
	pthread_t t2;
	
	pthread_create(&t1, NULL, func1, NULL);
	pthread_create(&t2, NULL, func2, NULL);
	
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	

	// inst_uninitialize();

	// second run is to execute active scheduler
	scheduler_activator();
	test();

	inst_initialize(exec_order, exec_length);
	
	pthread_t t3;
	pthread_t t4;
	
	pthread_create(&t3, NULL, func1, NULL);
	pthread_create(&t4, NULL, func2, NULL);
	
	pthread_join(t3, NULL);
	pthread_join(t4, NULL);

	inst_uninitialize();

	// for (int m = 0; m < thd1_index; ++m)
	// {
	// 	for (int n = 0; n < thd2_index; ++n)
	// 	{
	// 		if (thd1[m].accessed_mem_addr == thd2[n].accessed_mem_addr)
	// 		{
	// 			printf("%d, %d\n", thd1[m].thread_id*1000+thd1[m].instruction_id, thd2[n].thread_id*1000+thd2[n].instruction_id);
	// 		}
	// 	}
	// }

	// for (int i = 0; i < 4; ++i)
	// {
	// 	printf("%p\n", infos[i].accessed_mem_addr);
	// }

	exit(0);
}
