#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

int a = 0;

void* func(void* x) {
	a = 1;
	if (a==3)
		assert(1);
	return NULL;
}

int main() {
	pthread_t t;
	pthread_create(&t, NULL, func, NULL);
	
	a = 2;
	a = 3;

	pthread_join(t, NULL);
}
