#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

int a = 0;

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

int signaled1 = 0;
int signaled2 = 0;

void
begin(int index)
{
	fprintf(stderr, "begin(%d)\n", index);
	if (index == 1001) {
		// do nothing
	} else if (index == 2001) {
		pthread_mutex_lock(&lock1);
		while (!signaled2){
			pthread_cond_wait(&cond2, &lock1);
		}
		signaled2 = 0;
		pthread_mutex_unlock(&lock1);
	} else if (index == 1002) {
		pthread_mutex_lock(&lock2);
		while (!signaled1){
			pthread_cond_wait(&cond1, &lock2);
		}
		signaled1 = 0;
		pthread_mutex_unlock(&lock2);
	}
}

void
end(int index)
{
	fprintf(stderr, "end(%d)\n", index);
	if (index == 1001) {
		pthread_mutex_lock(&lock1);
		pthread_cond_signal(&cond2);
		signaled2 = 1;
		pthread_mutex_unlock(&lock1);
	} else if (index == 2001) {
		pthread_mutex_lock(&lock2);
		pthread_cond_signal(&cond1);
		signaled1 = 1;
		pthread_mutex_unlock(&lock2);
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
	pthread_t t1;
	pthread_t t2;
	
	pthread_create(&t1, NULL, func1, NULL);
	pthread_create(&t2, NULL, func2, NULL);
	
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	
	exit(0);
}
