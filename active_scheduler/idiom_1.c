#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "trigger.h"

#define EXEC_LENGTH 3

int exec_order[EXEC_LENGTH] = {1001, 2001, 1002};

int a = 0;

trigger *trigger1 = NULL;
trigger *trigger2 = NULL;

void
begin(int index)
{
	fprintf(stderr, "begin(%d)\n", index);
	if ((index != exec_order[0]) && (index/1000 == 2))
	{
		trigger_wait(trigger2);
	} else if ((index != exec_order[0]) && (index/1000 == 1))
	{
		trigger_wait(trigger1);
	}
}

void
end(int index)
{
	fprintf(stderr, "end(%d)\n", index);
	if ((index != exec_order[EXEC_LENGTH-1]) && (index/1000 == 1))
	{
		trigger_signal(trigger2);
	} else if ((index != exec_order[EXEC_LENGTH-1]) && (index/1000 == 2))
	{
		trigger_signal(trigger1);
	} 
}

// void
// begin(int index)
// {
// 	fprintf(stderr, "begin(%d)\n", index);
// 	if (index == exe_order[1]) {
// 		trigger_wait(trigger2);
// 	} else if (index == exe_order[2]) {
// 		trigger_wait(trigger1);
// 	}
// }

// void
// end(int index)
// {
// 	fprintf(stderr, "end(%d)\n", index);
// 	if (index == exe_order[0]) {
// 		trigger_signal(trigger2);
// 	} else if (index == exe_order[1]) {
// 		trigger_signal(trigger1);
// 	} 
// }

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
	a = 2;
	end(1002);
	return NULL;
}

void *
func2 (void *arg)
{
	begin(2001);
	if (a==1) {
		crash();
	}
	end(2001);
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
