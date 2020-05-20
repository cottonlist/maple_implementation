#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include "trigger.h"

int exec_order[] = {1001, 2001};
int exec_length = sizeof(exec_order)/sizeof(exec_order[0]);

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void *
func1(void *args)
{
	inst_begin(1001);
	pthread_mutex_lock(&mutex1);
	inst_end(1001);
	inst_begin(1002);
	pthread_mutex_lock(&mutex2);
	inst_end(1002);
	inst_begin(1003);
	printf("failed\n");
	inst_end(1003);
	inst_begin(1004);
	pthread_mutex_unlock(&mutex2);
	inst_end(1004);
	inst_begin(1005);
	pthread_mutex_unlock(&mutex1);
	inst_end(1005);

	return NULL;
}

void *
func2(void *args)
{	
	inst_begin(2001);
	pthread_mutex_lock(&mutex2);
	inst_end(2001);
	inst_begin(2002);
	pthread_mutex_lock(&mutex1);
	inst_end(2002);
	inst_begin(2003);
	printf("failed\n");
	inst_end(2003);
	inst_begin(2004);
	pthread_mutex_unlock(&mutex1);
	inst_end(2004);
	inst_begin(2005);
	pthread_mutex_unlock(&mutex2);
	inst_end(2005);

	return NULL;
}

int main() {
	inst_initialize(exec_order, exec_length);

	pthread_t t1;
	pthread_t t2;

	pthread_create(&t1, NULL, func1, NULL);
	pthread_create(&t2, NULL, func2, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	
	inst_uninitialize();

	exit(0);
}
