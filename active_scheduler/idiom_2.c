#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "trigger.h"

#define EXEC_LENGTH 4

int exec_order[EXEC_LENGTH] = {1001, 2001, 1002, 2002};

typedef struct
{	
	int score;
}student;

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

// void
// begin (int index)
// {
// 	fprintf(stderr, "begin(%d)\n", index);
// 	if (index == 1001){
// 		// do nothing
// 	} else if (index == 2001){
// 		trigger_wait(trigger2);
// 	} else if (index == 1002){
// 		trigger_wait(trigger1);
// 	} else if (index == 2002){
// 		trigger_wait(trigger2);
// 	}
// }

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
// end (int index)
// {
// 	fprintf(stderr, "end(%d)\n", index);
// 	if (index == 1001){
// 		trigger_signal(trigger2);
// 	} else if (index == 2001){
// 		trigger_signal(trigger1);
// 	} else if (index == 2002){
// 		// do nothing
// 	} else if (index == 1002){
// 		trigger_signal(trigger2);
// 	}
// }

void
crash()
{
	fprintf(stderr, "crash()\n");
}

void *
func1 (void *std)
{
	begin(1001);
	if (((student *)std)->score >= 60){
		end(1001);
		begin(1002);
		if (((student *)std)->score < 60){
			crash();
		}
		end(1002);
	}
	return NULL;
}

void *
func2 (void *std)
{
	begin(2001);
	((student *)std)->score = 30;
	//std = NULL;
	end(2001);
	begin(2002);
	((student *)std)->score = 80;
	end(2002);
	return NULL;
}

int
main()
{
	student taro = {80};

	student *student1 = &taro;

	trigger1 = trigger_init();
	trigger2 = trigger_init();

	pthread_t t1;
	pthread_t t2;

	pthread_create(&t1, NULL, func1, (void*)student1);
	pthread_create(&t2, NULL, func2, (void*)student1);
	
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	trigger_destroy(trigger1);
	trigger_destroy(trigger2);

	exit(0);
}
