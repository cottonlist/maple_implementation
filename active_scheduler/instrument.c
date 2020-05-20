#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "trigger.h"

trigger *trigger1;
trigger *trigger2;
int *current_exec_order;
int current_exec_length;

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
inst_begin(int index)
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

void
inst_end(int index)
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

void
crash()
{
	fprintf(stderr, "crash()\n");
}