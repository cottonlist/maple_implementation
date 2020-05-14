#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include "trigger.h"

#define EXEC_LENGTH 6

int exec_order[EXEC_LENGTH] = {2002, 1001, 2003, 2004, 1003, 2005};

trigger *trigger1 = NULL;
trigger *trigger2 = NULL;

int a = 0;
int b = 0;

void
begin(int index)
{
	fprintf(stderr, "begin(%d)\n", index);
	if (index == exec_order[1])
	{
		trigger_wait(trigger1);
	} else if (index == exec_order[2])
	{
		trigger_wait(trigger2);
	} else if (index == exec_order[4])
	{
		trigger_wait(trigger1);
	} else if (index == exec_order[5])
	{
		trigger_wait(trigger2);
	}
}

void
end(index)
{
	fprintf(stderr, "end(%d)\n", index);
	if (index == exec_order[0])
	{
		trigger_signal(trigger1);
	} else if (index == exec_order[1])
	{
		trigger_signal(trigger2);
	} else if (index == exec_order[3])
	{
		trigger_signal(trigger1);
	} else if (index == exec_order[4])
	{
		trigger_signal(trigger2);
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
	b = 1;
	end(1002);
	begin(1003);
	if (a == 2)
	{
		crash();
	}
	end(1003);

	return NULL;
}

void *
func2(void *arg)
{
	begin(2001);
	b = 2;
	end(2001);
	begin(2002);
	a = 3;
	end(2002);
	begin(2003);
	if (a == 1)
	{
		end(2003);
		begin(2004);
		a = 2;
		end(2004);
	}
	begin(2005);
	a = 4;
	end(2005);

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
