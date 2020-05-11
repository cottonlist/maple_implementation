#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

int a = 0;

pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t s1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t s2 = PTHREAD_COND_INITIALIZER;

int signaled1 = 0;
int signaled2 = 0;

void begin(int index)
{
	fprintf(stderr, "begin(%d)\n", index);
	if (index == 1001) {
		// do nothing
	} else if (index == 2001) {
		pthread_mutex_lock(&m1);
		while (!signaled2){
			pthread_cond_wait(&s2, &m1);
		}
		pthread_mutex_unlock(&m1);
	} else if (index == 1002) {
		pthread_mutex_lock(&m2);
		while (!signaled1){
			pthread_cond_wait(&s1, &m2);
		}
		pthread_mutex_unlock(&m2);
	}
}

void end(int index)
{
	fprintf(stderr, "end(%d)\n", index);
	if (index == 1001) {
		pthread_mutex_lock(&m1);
		pthread_cond_signal(&s2);
		signaled2 = 1;
		pthread_mutex_unlock(&m1);
	} else if (index == 2001) {
		pthread_mutex_lock(&m2);
		pthread_cond_signal(&s1);
		signaled1 = 1;
		pthread_mutex_unlock(&m2);
	}
}

void crash()
{
	fprintf(stderr, "crash()\n");
}

void* func1(void* x) {
	begin(1001);
	a = 1;
	end(1001);
	begin(1002);
	a = 2;
	end(1002);
	return NULL;
}

void * func2 (void * y)
{
	begin(2001);
	if (a==1) {
		crash();
	}
	end(2001);
	return NULL;
}

int main() {
	pthread_t t1, t2;
	pthread_create(&t1, NULL, func1, NULL);
	pthread_create(&t2, NULL, func2, NULL);
	
	//assert(a != 1);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
}
