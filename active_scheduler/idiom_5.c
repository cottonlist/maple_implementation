#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include "trigger.h"

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

trigger *trigger1 = NULL;
trigger *trigger2 = NULL;

void
begin(int index)
{
	fprintf(stderr, "begin(%d)\n", index);
	if (index == 2001)
	{
		trigger_wait(trigger2);
	} else if (index == 1002)
	{
		trigger_wait(trigger1);
	}
}

void
end(int index)
{
	fprintf(stderr, "end(%d)\n", index);
	if (index == 1001)
	{
		trigger_signal(trigger2);
	} else if (index == 2001)
	{
		trigger_signal(trigger1);
	}
}

void *
func1(void *args)
{
	begin(1001);
	pthread_mutex_lock(&mutex1);
	end(1001);
	begin(1002);
	pthread_mutex_lock(&mutex2);
	end(1002);
	begin(1003);
	printf("failed\n");
	end(1003);
	begin(1004);
	pthread_mutex_unlock(&mutex2);
	end(1004);
	begin(1005);
	pthread_mutex_unlock(&mutex1);
	end(1005);

	return NULL;
}

void *
func2(void *args)
{	
	begin(2001);
	pthread_mutex_lock(&mutex2);
	end(2001);
	begin(2002);
	pthread_mutex_lock(&mutex1);
	end(2002);
	begin(2003);
	printf("failed\n");
	end(2003);
	begin(2004);
	pthread_mutex_unlock(&mutex1);
	end(2004);
	begin(2005);
	pthread_mutex_unlock(&mutex2);
	end(2005);

	return NULL;
}

int main() {
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
