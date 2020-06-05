#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "trigger.h"
#include "instrumenter.h"

trigger *trigger1;
trigger *trigger2;
int *current_exec_order;
int current_exec_length;

// use mode variable to switch between profiler and scheduler
// 0 if profiler, 1 if active scheduler
static int phase;

struct info thd1[2];
struct info thd2[2];

// void
// profiler_initialize()
// {
// 	struct info thd1[2];
// 	struct info thd2[2];
// }

int thd1_index = 0;
int thd2_index = 0;

void activate_profiler()
{
	phase = 0;
}

void activate_scheduler()
{
	phase = 1;
}

void
inst_initialize(int *exec_order, int exec_length)
{
	trigger1 = trigger_init();
	trigger2 = trigger_init();
	current_exec_order = exec_order;
	current_exec_length = exec_length;
}

void
inst_uninitialize()
{
	trigger_destroy(trigger1);
	trigger_destroy(trigger2);
}

void
inst_begin(int index, int *accessed, int mode)
{
	if (phase == 0)
	{
		if (index / 1000 == 1)
		{
			thd1[thd1_index].thread_id = 1;
			thd1[thd1_index].instruction_id = index % 1000;
			thd1[thd1_index].accessed_mem_addr = accessed;
			thd1[thd1_index].mode = mode;
			thd1_index++;
		} else if (index / 1000 == 2)
		{
			thd2[thd2_index].thread_id = 2;
			thd2[thd2_index].instruction_id = index % 1000;
			thd2[thd2_index].accessed_mem_addr = accessed;
			thd2[thd2_index].mode = mode;
			thd2_index++;
		}
	}

	else
	{
		int isContained = 0;
		int previous;
		for (int i = 0; i < current_exec_length; ++i)
		{
			if (index == current_exec_order[i])
			{
				isContained = 1;
				previous = i - 1;
			}
		}
		fprintf(stderr, "begin(%d)\n", index);
		if (index != current_exec_order[0] && isContained == 1)
		{
			if (index/1000 == 2 && current_exec_order[previous]/1000 == 1)
			{
				trigger_wait(trigger2);
			} else if (index/1000 == 1 && current_exec_order[previous]/1000 == 2)
			{
				trigger_wait(trigger1);
			}
		}
	}
}

void
inst_end(int index)
{
	if (phase == 1)
	{
		int isContained = 0;
		int next;
		for (int i = 0; i < current_exec_length; ++i)
		{
			if (index == current_exec_order[i])
			{
				isContained = 1;
				next = i + 1;
			}
		}
		fprintf(stderr, "end(%d)\n", index);
		if (index != current_exec_order[current_exec_length-1] && isContained == 1)
		{
			if (index/1000 == 1 && current_exec_order[next]/1000 == 2)
			{
				trigger_signal(trigger2);
			} else if (index/1000 == 2 && current_exec_order[next]/1000 == 1)
			{
				trigger_signal(trigger1);
			}
		}
	}
}

void test()
{
	printf("mode is %d\n", phase);
	for (int i = 0; i < thd1_index; ++i)
	{
		printf("%d\t%d\t%p\t%d\n", thd1[i].thread_id, thd1[i].instruction_id, thd1[i].accessed_mem_addr, thd1[i].mode);
	}
	for (int i = 0; i < thd2_index; ++i)
	{
		printf("%d\t%d\t%p\t%d\n", thd2[i].thread_id, thd2[i].instruction_id, thd2[i].accessed_mem_addr, thd2[i].mode);
	}
}

void
crash()
{
	fprintf(stderr, "crash()\n");
}
