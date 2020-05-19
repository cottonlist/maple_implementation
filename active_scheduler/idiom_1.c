#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "trigger.h"

// int exec_order[] = {1001, 2001, 1002};
// int exec_order[] = {2001, 1002, 2003, 1004};
// int exec_order[] = {1002, 2003, 1003, 2004};
// int exec_order[] = {2002, 1002, 2003, 1003, 2004, 1004};
// int exec_order[] = {1001, 2003, 2004, 1004, 2005};
// int exec_order[] = {2002, 2003, 1001, 2004, 1003, 1004};
int exec_order[] = {2002, 2003, 2004, 1002, 1003, 2005};


int exec_length = sizeof(exec_order)/sizeof(exec_order[0]);

// int exec_length = sizeof(exec_order);

int a = 0;

trigger *trigger1 = NULL;
trigger *trigger2 = NULL;

void
begin(int index)
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
end(int index)
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

void
crash()
{
	fprintf(stderr, "crash()\n");
}

void *
func1(void *arg)
{
	begin(1001);
	a = 1;
	end(1001);

	begin(1002);
	a = 1;
	end(1002);

	begin(1003);
	a = 1;
	end(1003);

	begin(1004);
	a = 1;
	end(1004);

	begin(1005);
	a = 1;
	end(1005);

	// begin(1001);
	// a = 1;
	// end(1001);
	// begin(1002);
	// a = 2;
	// end(1002);
	return NULL;
}

void *
func2 (void *arg)
{
	begin(2001);
	a = 1;
	end(2001);

	begin(2002);
	a = 1;
	end(2002);

	begin(2003);
	a = 1;
	end(2003);

	begin(2004);
	a = 1;
	end(2004);

	begin(2005);
	a = 1;
	end(2005);
	// begin(2001);
	// if (a==1) {
	// 	crash();
	// }
	// end(2001);
	return NULL;
}

int
main(void)
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
