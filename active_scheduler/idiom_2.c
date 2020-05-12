#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

typedef struct
{	
	int score;
}student;

void *
func1 (student *std)
{
	if (std)
	{
		printf("%d", std->score);
	}
	return NULL;
}

void *
func2 (student *std)
{
	std = NULL;
	return NULL;
}

int main() {
	student taro = {80};
	student *student1 = &taro;

	pthread_t t1;
	pthread_t t2;

	pthread_create(&t1, NULL, func1, student1);
	pthread_create(&t2, NULL, func2, student1);
	
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	exit(0);
}
