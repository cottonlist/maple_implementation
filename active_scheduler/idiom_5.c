#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include "trigger.h"
#include "instrumenter.h"

pthread_mutex_t lock;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void *
func1(void *args)
{
	inst_begin(1001, SYNC, &mutex1, MODE_LOCK, NULL, 0, 0);
	pthread_mutex_lock(&mutex1);
	inst_end(1001);

	inst_begin(1002, SYNC, &mutex2, MODE_LOCK, mutex1, 1, 0);
	pthread_mutex_lock(&mutex2);
	inst_end(1002);

	inst_begin(1003, OTHER, NULL, MODE_READ, mutex2, 1, 1);
	printf("func1\n");
	inst_end(1003);

	inst_begin(1004, SYNC, &mutex2, MODE_UNLOCK, mutex1, 0, 1);
	pthread_mutex_unlock(&mutex2);
	inst_end(1004);

	inst_begin(1005, SYNC, &mutex1, MODE_UNLOCK, NULL, 0, 0);
	pthread_mutex_unlock(&mutex1);
	inst_end(1005);

	return NULL;
}

void *
func2(void *args)
{	
	inst_begin(2001, SYNC, &mutex2, MODE_LOCK, NULL, 0, 0);
	pthread_mutex_lock(&mutex2);
	inst_end(2001);

	inst_begin(2002, SYNC, &mutex1, MODE_LOCK, mutex1, 1, 0);
	pthread_mutex_lock(&mutex1);
	inst_end(2002);

	inst_begin(2003, OTHER, NULL, MODE_READ, mutex1, 1, 1);
	printf("func2\n");
	inst_end(2003);

	inst_begin(2004, SYNC, &mutex1, MODE_UNLOCK, mutex1, 0, 1);
	pthread_mutex_unlock(&mutex1);
	inst_end(2004);

	inst_begin(2005, SYNC, &mutex2, MODE_UNLOCK, NULL, 0, 0);
	pthread_mutex_unlock(&mutex2);
	inst_end(2005);

	return NULL;
}

int main() {
	pthread_mutex_init(&lock, NULL);
	// run profiler
	activate_profiler();
	pthread_t t1;
	pthread_t t2;
	pthread_create(&t1, NULL, func1, NULL);
	pthread_create(&t2, NULL, func2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	create_exec_order(5);
	activate_scheduler();

	// run active scheduler
	test(t1, t2, func1, func2);

	pthread_mutex_destroy(&lock);

	exit(0);
}
