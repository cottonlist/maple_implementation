#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include "trigger.h"

// Execution order provided by profiler
int exec_order[] = {2002, 1001, 2003, 2004, 1003, 2005};
int exec_length = sizeof(exec_order)/sizeof(exec_order[0]);


trigger *trigger1 = NULL;
trigger *trigger2 = NULL;

int a = 0;
int b = 0;

void
crash()
{
	fprintf(stderr, "crash()\n");
}

void *
func1(void *arg)
{
	inst_begin(1001, exec_order, exec_length, trigger1, trigger2);
	a = 1;
	inst_end(1001, exec_order, exec_length, trigger1, trigger2);
	inst_begin(1002, exec_order, exec_length, trigger1, trigger2);
	b = 1;
	inst_end(1002, exec_order, exec_length, trigger1, trigger2);
	inst_begin(1003, exec_order, exec_length, trigger1, trigger2);
	if (a == 2)
	{
		crash();
	}
	inst_end(1003, exec_order, exec_length, trigger1, trigger2);

	return NULL;
}

void *
func2(void *arg)
{
	inst_begin(2001, exec_order, exec_length, trigger1, trigger2);
	b = 2;
	inst_end(2001, exec_order, exec_length, trigger1, trigger2);
	inst_begin(2002, exec_order, exec_length, trigger1, trigger2);
	a = 3;
	inst_end(2002, exec_order, exec_length, trigger1, trigger2);
	inst_begin(2003, exec_order, exec_length, trigger1, trigger2);
	if (a == 1)
	{
		inst_end(2003, exec_order, exec_length, trigger1, trigger2);
		inst_begin(2004, exec_order, exec_length, trigger1, trigger2);
		a = 2;
		inst_end(2004, exec_order, exec_length, trigger1, trigger2);
	}
	inst_begin(2005, exec_order, exec_length, trigger1, trigger2);
	a = 4;
	inst_end(2005, exec_order, exec_length, trigger1, trigger2);

	return NULL;
}

int
main()
{
	trigger1 = trigger_init();
	trigger2 = trigger_init();

	pthread_t t1;
	pthread_t t2;

	pthread_create(&t1, NULL, func1, NULL);
	pthread_create(&t2, NULL, func2, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	trigger_destroy(trigger1);
	trigger_destroy(trigger2);

	exit(0);
}
