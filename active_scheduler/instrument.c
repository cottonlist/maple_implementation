#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "trigger.h"

void
inst_begin(int index, int exec_order[], int exec_length, trigger *trigger1, trigger *trigger2)
{
	int isContained = 0;
	int previous;
	for (int i = 0; i < exec_length; ++i)
	{
		if (index == exec_order[i])
		{
			isContained = 1;
			previous = i - 1;
		}
	}
	fprintf(stderr, "begin(%d)\n", index);
	if (index != exec_order[0] && isContained == 1)
	{
		if (index/1000 == 2 && exec_order[previous]/1000 == 1)
		{
			trigger_wait(trigger2);
		} else if (index/1000 == 1 && exec_order[previous]/1000 == 2)
		{
			trigger_wait(trigger1);
		}
	}
}

void
inst_end(int index, int exec_order[], int exec_length, trigger *trigger1, trigger *trigger2)
{
	int isContained = 0;
	int next;
	for (int i = 0; i < exec_length; ++i)
	{
		if (index == exec_order[i])
		{
			isContained = 1;
			next = i + 1;
		}
	}
	fprintf(stderr, "end(%d)\n", index);
	if (index != exec_order[exec_length-1] && isContained == 1)
	{
		if (index/1000 == 1 && exec_order[next]/1000 == 2)
		{
			trigger_signal(trigger2);
		} else if (index/1000 == 2 && exec_order[next]/1000 == 1)
		{
			trigger_signal(trigger1);
		}
	}
}