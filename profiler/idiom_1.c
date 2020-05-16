#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define numOfInstructions 4
#define numOfInstances 2

typedef struct {
	int threadId;
	int instructionId;
	char accessedVariable[25];
	int isRead;
	int isWrite;
}instructionInfo;

void *
instructInfoInit(int thr, int ins, char var[], int isRead, int isWrite)
{
	instructionInfo *info = malloc(sizeof *info);
	info->threadId = thr;
	info->instructionId = ins;
	strcpy(info->accessedVariable, var);
	info->isRead = isRead;
	info->isWrite = isWrite;

	return info;
}


instructionInfo *infos[numOfInstructions];

int a = 0;

void
crash()
{
	fprintf(stderr, "crash()\n");
}

void *
func1(void *arg)
{
	a = 1;
	infos[0] = instructInfoInit(1, 1, "a", 1, 0);
	a = 2;
	infos[1] = instructInfoInit(1, 2, "a", 1, 0);
	return NULL;
}

void *
func2 (void *arg)
{
	a = 3;
	infos[2] = instructInfoInit(2, 1, "a", 1, 0);
	if (a==1) {
		crash();
	}
	infos[3] = instructInfoInit(2, 2, "a", 0, 1);
	return NULL;
}

int
main(void)
{

	for (int i = 0; i < numOfInstructions; ++i)
	{
	infos[i] = malloc(sizeof(instructionInfo));
	}

	pthread_t t1;
	pthread_t t2;
	
	pthread_create(&t1, NULL, func1, NULL);
	pthread_create(&t2, NULL, func2, NULL);
	
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	int instanceCounter = 0;

	for (int i = 0; i < numOfInstructions; ++i)
	{
		for (int j = i+1; j < numOfInstructions; ++j)
		{
			if ((infos[i]->threadId != infos[j]->threadId) && 
				(strcmp(infos[i]->accessedVariable, infos[j]->accessedVariable)==0) &&
				(infos[i]->isRead != infos[j]->isRead))
			{
				printf("Idiom 1 instance found:\n");
				printf("Thread %d, instruction %d.\n", infos[i]->threadId, infos[i]->instructionId);
				printf("Thread %d, instruction %d.\n", infos[j]->threadId, infos[j]->instructionId);
				int execOrder[4] = {infos[j]->threadId*1000+infos[j]->instructionId-1, infos[i]->threadId*1000+infos[i]->instructionId, infos[j]->threadId*1000+infos[j]->instructionId, infos[i]->threadId*1000+infos[i]->instructionId+1};
				for (int i = 0; i < 4; ++i)
				{
					printf("%d\n", execOrder[i]);
				}
				//execOrder1 = {0, 0, 0, 0};
				//execOrder1 = {0, (infos[i]->threadId*1000)+(infos[i]->instructionId), (infos[j]->threadId)*1000+(infos[j]->instructionId), 0};
			}
		}
	}

	exit(0);
}
