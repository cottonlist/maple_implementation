#include <stdio.h>
#include <assert.h>
#include <pthread.h>

int a = 0;
int b = 0;

void* foo(void* arg) {
	a = 1;
	if(a == 2) {
		b = 1;
	}
	return NULL;
}

void* bar(void* arg) {
	a = 2;
	assert(b != 1);
	return NULL;
}

int main() {
	pthread_t t1;
	pthread_t t2;

	pthread_create(&t1, NULL, foo, NULL);
	pthread_create(&t2, NULL, bar, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	return 0;
}
