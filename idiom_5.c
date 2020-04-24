#include <stdio.h>
#include <pthread.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* foo(void* args){
	pthread_mutex_lock(&mutex1);
	pthread_mutex_lock(&mutex2);
	printf("foo\n");
	pthread_mutex_lock(&mutex2);
	pthread_mutex_lock(&mutex1);
	return NULL;
}

void* bar(void* args){	
	pthread_mutex_lock(&mutex2);
	pthread_mutex_lock(&mutex1);
	printf("bar\n");
	pthread_mutex_lock(&mutex1);
	pthread_mutex_lock(&mutex2);
	return NULL;
}

int main() {
	pthread_t t1, t2;
	pthread_create(&t1, NULL, foo, NULL);
	pthread_create(&t2, NULL, bar, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	
	return 0;
}
