#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

int a = 0;

typedef struct {
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int isSignaled;
} trigger;



trigger *
trigger_init()
{
	trigger *t = (trigger *) malloc(sizeof(trigger));
	//trigger *t = (trigger *) calloc(1, sizeof(trigger));
	
	pthread_mutex_init(&(t->lock), NULL);
	pthread_cond_init(&(t->cond), NULL);
	
	//t->lock = PTHREAD_MUTEX_INITIALIZER;
	//t->cond = PTHREAD_COND_INITIALIZER;
	t->isSignaled = 0;
	
	return t;
}

void
trigger_destroy(trigger *t)
{
	pthread_mutex_destroy(&(t->lock));
	pthread_cond_destroy(&(t->cond));
	
	free(t);
}

void
trigger_signal(trigger *t)
{
	pthread_mutex_lock(&(t->lock));
	pthread_cond_signal(&(t->cond));
	t->isSignaled = 1;
	pthread_mutex_unlock(&(t->lock));
}

void
trigger_wait(trigger *t)
{
	pthread_mutex_lock(&(t->lock));
	while (!t->isSignaled){
		pthread_cond_wait(&(t->cond), &(t->lock));
	}
	t->isSignaled = 0;
	pthread_mutex_unlock(&(t->lock));
}

trigger *trigger1 = NULL;
trigger *trigger2 = NULL;

void
begin(int index)
{
	fprintf(stderr, "begin(%d)\n", index);
	if (index == 1001) {
		// do nothing
	} else if (index == 2001) {
		trigger_wait(trigger2);
	} else if (index == 1002) {
		trigger_wait(trigger1);
	}
}

void
end(int index)
{
	fprintf(stderr, "end(%d)\n", index);
	if (index == 1001) {
		trigger_signal(trigger2);
	} else if (index == 2001) {
		trigger_signal(trigger1);
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
