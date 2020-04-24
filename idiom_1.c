#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

int a = 0;

void* func(void* x) {
	a = 1;
	a = 2;
	return NULL;
}

int main() {
	pthread_t t;
	pthread_create(&t, NULL, func, NULL);
	
	assert(a != 1);

	pthread_join(t, NULL);
}
