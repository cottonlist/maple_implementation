#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "trigger.h"
#include "instrumenter.h"

trigger *trigger1;
trigger *trigger2;
int **current_exec_order;
int current_exec_length;
int num_of_exec_order = 0;

FILE *fp;

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

void create_exec_order()
{
	if ((fp = fopen(FILENAME, "a")) == NULL)
	{
		perror("fopen");
		exit(1);
	}

	for (int i = 0; i < thd1_index; ++i)
	{
		for (int j = 0; j < thd2_index; ++j)
		{
			if (thd1[i].accessed_mem_addr == thd2[j].accessed_mem_addr)
			{
				fprintf(fp, "One of exec_order is %d, %d\n", thd1[i].thread_id*1000+thd1[i].instruction_id, thd2[j].thread_id*1000+thd2[j].instruction_id);
				num_of_exec_order++;
				fprintf(fp, "One of exec_order is %d, %d\n", thd2[j].thread_id*1000+thd2[j].instruction_id, thd1[i].thread_id*1000+thd1[i].instruction_id);
				num_of_exec_order++;
			}
		}
	}

	if (fclose(fp) == EOF)
	{
		perror("fclose");
		exit(1);
	}
	
}

void init_exec_order()
{
	current_exec_order = (int**) malloc(sizeof(int) * 2 * num_of_exec_order); // multiply by 2 because idiom1 only involves 2 INS

	if ((fp = fopen(FILENAME, "r")) == NULL)
	{
		perror("fopen");
		exit(1);
	}

	for (int i = 0; i < num_of_exec_order; ++i)
	{
		/* initialize all exec_order */
	}

	free(current_exec_order);

	if (fclose(fp) == EOF)
	{
		perror("fclose");
		exit(1);
	}
}

void
crash()
{
	fprintf(stderr, "crash()\n");
}
