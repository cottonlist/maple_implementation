#include <stdio.h>
#include <pthread.h>

#include "trigger.h"
#include "instrumenter.h"

pthread_mutex_t lock;
pthread_mutex_t l;

static int a;
static int b;
static int c;

struct student {
	int score;
};

static struct student taro;
static struct student *student1;

void *func1(void *arg){

	inst_begin(1001, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 1;
	inst_end(1001);

	inst_begin(1002, VAR, &c, MODE_WRITE, NULL, 0, 0);
	pthread_mutex_lock(&l);
	student1 = &taro;
	pthread_mutex_unlock(&l);
	inst_end(1002);

	// Happening of segmentation fault here means manifestation of bug. 
	inst_begin(1003, VAR, &c, MODE_READ, NULL, 0, 0);
	pthread_mutex_lock(&l);
	printf("Taro's score is %d\n", student1->score);
	pthread_mutex_unlock(&l);
	inst_end(1003);

	inst_begin(1004, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 1;
	inst_end(1004);

	return NULL;
}

void *func2(void *arg){

	inst_begin(2001, VAR, &a, MODE_WRITE, NULL, 0, 0);
	a = 2;
	inst_end(2001);

	inst_begin(2002, VAR, &c, MODE_WRITE, NULL, 0, 0);
	pthread_mutex_lock(&l);
	student1 = NULL;
	pthread_mutex_unlock(&l);
	inst_end(2002);

	inst_begin(2003, VAR, &b, MODE_WRITE, NULL, 0, 0);
	b = 2;
	inst_end(2003);

	return NULL;
}

int main(int argc, char const *argv[])
{
	pthread_mutex_init(&lock, NULL);
	activate_profiler();
	pthread_t t1;
	pthread_t t2;
	pthread_create(&t1, NULL, func1, NULL);
	pthread_create(&t2, NULL, func2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	create_exec_order(2);
	activate_scheduler();
	test(t1, t2, func1, func2);
	pthread_mutex_destroy(&lock);

	return 0;
}