#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include "trigger.h"

int exec_order[] = {1001, 2001};
int exec_length = sizeof(exec_order)/sizeof(exec_order[0]);

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

trigger *trigger1 = NULL;
trigger *trigger2 = NULL;

void *
func1(void *args)
{
	inst_begin(1001, exec_order, exec_length, trigger1, trigger2);
	pthread_mutex_lock(&mutex1);
	inst_end(1001, exec_order, exec_length, trigger1, trigger2);
	inst_begin(1002, exec_order, exec_length, trigger1, trigger2);
	pthread_mutex_lock(&mutex2);
	inst_end(1002, exec_order, exec_length, trigger1, trigger2);
	inst_begin(1003, exec_order, exec_length, trigger1, trigger2);
	printf("failed\n");
	inst_end(1003, exec_order, exec_length, trigger1, trigger2);
	inst_begin(1004, exec_order, exec_length, trigger1, trigger2);
	pthread_mutex_unlock(&mutex2);
	inst_end(1004, exec_order, exec_length, trigger1, trigger2);
	inst_begin(1005, exec_order, exec_length, trigger1, trigger2);
	pthread_mutex_unlock(&mutex1);
	inst_end(1005, exec_order, exec_length, trigger1, trigger2);

	return NULL;
}

void *
func2(void *args)
{	
	inst_begin(2001, exec_order, exec_length, trigger1, trigger2);
	pthread_mutex_lock(&mutex2);
	inst_end(2001, exec_order, exec_length, trigger1, trigger2);
	inst_begin(2002, exec_order, exec_length, trigger1, trigger2);
	pthread_mutex_lock(&mutex1);
	inst_end(2002, exec_order, exec_length, trigger1, trigger2);
	inst_begin(2003, exec_order, exec_length, trigger1, trigger2);
	printf("failed\n");
	inst_end(2003, exec_order, exec_length, trigger1, trigger2);
	inst_begin(2004, exec_order, exec_length, trigger1, trigger2);
	pthread_mutex_unlock(&mutex1);
	inst_end(2004, exec_order, exec_length, trigger1, trigger2);
	inst_begin(2005, exec_order, exec_length, trigger1, trigger2);
	pthread_mutex_unlock(&mutex2);
	inst_end(2005, exec_order, exec_length, trigger1, trigger2);

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
