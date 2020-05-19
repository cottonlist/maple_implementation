// This is trigger.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "trigger.h"

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




