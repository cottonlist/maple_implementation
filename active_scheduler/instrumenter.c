#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "trigger.h"
#include "instrumenter.h"

trigger *trigger1;
trigger *trigger2;
int *exec_order[50];
int size[50];

// indicates which execution order
int which = 0;

// indicates number of execution orders created by profiling phase
int num = 0;

int *current_order;
int current_length;
int num_of_exec_order = 0;

FILE *fp;

// use mode variable to switch between profiler and scheduler
// 0 if profiler, 1 if active scheduler
static int phase;

struct info thd1[2];
struct info thd2[2];

int thd1_index = 0;
int thd2_index = 0;



void activate_profiler()
{
	phase = PROFILER;
}

void activate_scheduler()
{
	phase = SCHEDULER;
}

void
inst_initialize()
{
	trigger1 = trigger_init();
	trigger2 = trigger_init();
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
	if (phase == PROFILER)
	{
		if (index / 1000 == 1)
		{
			thd1[thd1_index].index = index;
			thd1[thd1_index].thread_id = 1;
			thd1[thd1_index].instruction_id = index % 1000;
			thd1[thd1_index].accessed_mem_addr = accessed;
			thd1[thd1_index].mode = mode;
			thd1_index++;
		} else if (index / 1000 == 2)
		{
			thd2[thd2_index].index = index;
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
		for (int i = 0; i < current_length; ++i)
		{
			if (index == current_order[i])
			{
				isContained = 1;
				previous = i - 1;
			}
		}
		fprintf(stderr, "begin(%d)\n", index);
		if (index != current_order[0] && isContained == 1)
		{
			if (index/1000 == 2 && current_order[previous]/1000 == 1)
			{
				trigger_wait(trigger2);
			} else if (index/1000 == 1 && current_order[previous]/1000 == 2)
			{
				trigger_wait(trigger1);
			}
		}
	}
}

void
inst_end(int index)
{
	if (phase == SCHEDULER)
	{
		int isContained = 0;
		int next;
		for (int i = 0; i < current_length; ++i)
		{
			if (index == current_order[i])
			{
				isContained = 1;
				next = i + 1;
			}
		}
		fprintf(stderr, "end(%d)\n", index);
		if (index != current_order[current_length-1] && isContained == 1)
		{
			if (index/1000 == 1 && current_order[next]/1000 == 2)
			{
				trigger_signal(trigger2);
			} else if (index/1000 == 2 && current_order[next]/1000 == 1)
			{
				trigger_signal(trigger1);
			}
		}
	}
}

void create_exec_order()
{
	// clean file
	if ((fp = fopen(FILENAME, "w")) == NULL)
	{
		perror("fopen");
		exit(1);
	}

	if (fclose(fp) == EOF)
	{
		perror("fclose");
		exit(1);
	}

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
				if (thd1[i].instruction_id == 1 && thd2[j].instruction_id == thd2_index)
				{
					fprintf(fp, "%d, %d, %d, %d\n", thd2[j].index-1, thd1[i].index, thd2[j].index, thd1[i].index+1);
					size[num] = 4;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd2[j].index - 1;
					exec_order[num][1] = thd1[i].index;
					exec_order[num][2] = thd2[j].index;
					exec_order[num][3] = thd1[i].index + 1;
					num++;
					fprintf(fp, "%d, %d\n", thd2[j].index, thd1[i].index);
					size[num] = 2;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd2[j].index;
					exec_order[num][1] = thd1[i].index;
					num++;
				} else if (thd1[i].instruction_id == thd1_index && thd2[j].instruction_id == 1)
				{
					fprintf(fp, "%d, %d, %d, %d\n", thd1[i].index-1, thd2[j].index, thd1[i].index, thd2[j].index+1);
					size[num] = 4;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd1[i].index - 1;
					exec_order[num][1] = thd2[j].index;
					exec_order[num][2] = thd1[i].index;
					exec_order[num][3] = thd2[j].index + 1;
					num++;
					fprintf(fp, "%d, %d\n", thd1[i].index, thd2[j].index);
					size[num] = 2;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd1[i].index;
					exec_order[num][1] = thd2[j].index;
					num++;
				} else if (thd1[i].instruction_id == 1 && thd2[j].instruction_id == 1)
				{
					fprintf(fp, "%d, %d, %d\n", thd1[i].index, thd2[j].index, thd1[i].index + 1);
					size[num] = 3;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd1[i].index;
					exec_order[num][1] = thd2[j].index;
					exec_order[num][2] = thd1[i].index + 1;
					num++;
					fprintf(fp, "%d, %d, %d\n", thd2[j].index, thd1[i].index, thd2[j].index+1);
					size[num] = 3;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd2[j].index;
					exec_order[num][1] = thd1[i].index;
					exec_order[num][2] = thd2[j].index + 1;
					num++;
				} else if (thd1[i].instruction_id == thd1_index && thd2[j].instruction_id == thd2_index)
				{
					fprintf(fp, "%d, %d, %d\n", thd1[i].index-1, thd2[j].index, thd1[i].index);
					size[num] = 3;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd1[i].index - 1;
					exec_order[num][1] = thd2[j].index;
					exec_order[num][2] = thd1[i].index;
					num++;
					fprintf(fp, "%d, %d, %d\n", thd2[j].index-1, thd1[i].index, thd2[j].index);
					size[num] = 3;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd2[j].index - 1;
					exec_order[num][1] = thd1[i].index;
					exec_order[num][2] = thd2[j].index;
					num++;
				} else if (thd1[i].instruction_id == 1 && thd2[j].instruction_id != 1 && thd2[j].instruction_id != thd2_index)
				{
					fprintf(fp, "%d, %d, %d, %d\n", thd2[j].index-1, thd1[i].index, thd2[j].index, thd1[i].index+1);
					size[num] = 4;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd2[j].index - 1;
					exec_order[num][1] = thd1[i].index;
					exec_order[num][2] = thd2[j].index;
					exec_order[num][3] = thd1[i].index + 1;
					num++;
					fprintf(fp, "%d, %d, %d\n", thd2[j].index, thd1[i].index, thd2[j].index+1);
					size[num] = 3;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd2[j].index;
					exec_order[num][1] = thd1[i].index;
					exec_order[num][2] = thd2[j].index + 1;
					num++;
				} else if (thd2[j].instruction_id == 1 && thd1[i].instruction_id != 1 && thd1[i].instruction_id != thd1_index)
				{
					fprintf(fp, "%d, %d, %d, %d\n", thd1[i].index-1, thd2[j].index, thd1[i].index, thd2[j].index+1);
					size[num] = 4;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd1[i].index - 1;
					exec_order[num][1] = thd2[j].index;
					exec_order[num][2] = thd1[i].index;
					exec_order[num][3] = thd2[j].index + 1;
					num++;
					fprintf(fp, "%d, %d, %d\n", thd1[i].index, thd2[j].index, thd1[i].index+1);
					size[num] = 3;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd1[i].index;
					exec_order[num][1] = thd2[j].index;
					exec_order[num][2] = thd1[i].index + 1;
					num++;
				} else if (thd1[i].instruction_id == thd1_index && thd2[j].instruction_id != 1 && thd2[j].instruction_id != thd2_index)
				{
					fprintf(fp, "%d, %d, %d, %d\n", thd1[i].index-1, thd2[j].index, thd1[i].index, thd2[j].index+1);
					size[num] = 4;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd1[i].index - 1;
					exec_order[num][1] = thd2[j].index;
					exec_order[num][2] = thd1[i].index;
					exec_order[num][3] = thd2[j].index + 1;
					num++;
					fprintf(fp, "%d, %d, %d\n", thd2[j].index-1, thd1[i].index, thd2[j].index);
					size[num] = 3;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd2[j].index - 1;
					exec_order[num][1] = thd1[i].index;
					exec_order[num][2] = thd2[j].index;
					num++;
				} else if (thd2[j].instruction_id == thd2_index && thd1[i].index != 1 && thd1[i].index != thd1_index)
				{
					fprintf(fp, "%d, %d, %d, %d\n", thd2[j].index-1, thd1[i].index, thd2[j].index, thd1[i].index+1);
					size[num] = 4;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd2[j].index - 1;
					exec_order[num][1] = thd1[i].index;
					exec_order[num][2] = thd2[j].index;
					exec_order[num][3] = thd1[i].index + 1;
					num++;
					fprintf(fp, "%d, %d, %d\n", thd1[i].index-1, thd2[j].index, thd1[i].index);
					size[num] = 3;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd1[i].index - 1;
					exec_order[num][1] = thd2[j].index;
					exec_order[num][2] = thd1[i].index;
					num++;
				} else
				{
					fprintf(fp, "%d, %d, %d, %d\n", thd2[j].index-1, thd1[i].index, thd2[j].index, thd1[i].index+1);
					size[num] = 4;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd2[j].index - 1;
					exec_order[num][1] = thd1[i].index;
					exec_order[num][2] = thd2[j].index;
					exec_order[num][3] = thd1[i].index + 1;
					num++;
					fprintf(fp, "%d, %d, %d, %d\n", thd1[i].index-1, thd2[j].index, thd1[i].index, thd2[j].index+1);
					size[num] = 4;
					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
					exec_order[num][0] = thd1[i].index - 1;
					exec_order[num][1] = thd2[j].index;
					exec_order[num][2] = thd1[i].index;
					exec_order[num][3] = thd2[j].index + 1;
					num++;
				}
				num_of_exec_order += 2;
			}
		}
	}

	if (fclose(fp) == EOF)
	{
		perror("fclose");
		exit(1);
	}
}

void assign_order()
{
	current_order = malloc(sizeof(int) * size[which]);
	for (int i = 0; i < size[which]; ++i)
	{
		current_order[i] = exec_order[which][i];
	}
	current_length = size[which];
	which++;
	free(current_order);
}

void test(pthread_t t1, pthread_t t2, void *func1, void *func2)
{	
	for (int i = 0; i < num; ++i)
	{	
		printf("Execution %d:\n", i + 1);
		assign_order();
		inst_initialize();
		pthread_create(&t1, NULL, func1, NULL);
		pthread_create(&t2, NULL, func2, NULL);
		pthread_join(t1, NULL);
		pthread_join(t2, NULL);
		inst_uninitialize();
		printf("\n");
	}
}

void
crash()
{
	fprintf(stderr, "crash()\n");
}
