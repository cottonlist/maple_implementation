#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct
{	
	int score;
}student;

typedef struct
{
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int isSignaled;
} trigger;

trigger *
trigger_init()
{
	trigger *t = (trigger *) malloc(sizeof(trigger));

	pthread_mutex_init(&(t->lock), NULL);
	pthread_cond_init(&(t->cond), NULL);
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
	while (!t->isSignaled)
	{
		pthread_cond_wait(&(t->cond), &(t->lock));
	}
	t->isSignaled = 0;
	pthread_mutex_unlock(&(t->lock));
}

trigger *trigger1 = NULL;
trigger *trigger2 = NULL;

void
begin (int index)
{
	fprintf(stderr, "begin(%d)\n", index);
	if (index == 1001){
		// do nothing
	} else if (index == 2001){
		trigger_wait(trigger2);
	} else if (index == 1002){
		trigger_wait(trigger1);
	} else if (index == 2002){
		trigger_wait(trigger2);
	}
}

void
end (int index)
{
	fprintf(stderr, "end(%d)\n", index);
	if (index == 1001){
		trigger_signal(trigger2);
	} else if (index == 2001){
		trigger_signal(trigger1);
	} else if (index == 2002){
		// do nothing
	} else if (index == 1002){
		trigger_signal(trigger2);
	}
}

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

int main()
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
