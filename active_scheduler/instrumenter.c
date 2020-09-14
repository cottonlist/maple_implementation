#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "trigger.h"
#include "instrumenter.h"

trigger *trigger1;
trigger *trigger2;
int *exec_order[200];
int size[10];

// indicates which execution order
int which = 0;

// indicates number of execution orders created by profiling phase
int num = 0;

int *current_order;
int current_length;
int num_of_exec_order = 0;

FILE *fp;

// use mode variable to switch between profiler and scheduler
// 0 if profiler, 1 if active scheduler
static int phase;

struct info thd1[50];
struct info thd2[50];

int thd1_index = 0;
int thd2_index = 0;

void activate_profiler()
{
	phase = PROFILER;
}

void activate_scheduler()
{
	phase = SCHEDULER;
}

void
inst_initialize()
{
	trigger1 = trigger_init();
	trigger2 = trigger_init();
}

void
inst_uninitialize()
{
	trigger_destroy(trigger1);
	trigger_destroy(trigger2);
}

void
inst_begin(int index, int type, int *accessed, int mode, pthread_mutex_t *lock, int first, int last)
{
	if (phase == PROFILER)
	{
		if (index / 1000 == 1)
		{
			thd1[thd1_index].index = index;
			thd1[thd1_index].type = type;
			thd1[thd1_index].thread_id = 1;
			thd1[thd1_index].instruction_id = index % 1000;
			thd1[thd1_index].accessed_mem_addr = accessed;
			thd1[thd1_index].mode = mode;
			thd1[thd1_index].lock = lock;
			thd1[thd1_index].first_in_mutex = first;
			thd1[thd1_index].last_in_mutex = last;
			thd1_index++;
		} else if (index / 1000 == 2)
		{
			thd2[thd2_index].index = index;
			thd2[thd2_index].type = type;
			thd2[thd2_index].thread_id = 2;
			thd2[thd2_index].instruction_id = index % 1000;
			thd2[thd2_index].accessed_mem_addr = accessed;
			thd2[thd2_index].mode = mode;
			thd2[thd2_index].lock = lock;
			thd2[thd2_index].first_in_mutex = first;
			thd2[thd2_index].last_in_mutex = last;
			thd2_index++;
		}
	}

	else
	{
		int isContained = 0;
		int previous;
		for (int i = 0; i < current_length; ++i)
		{
			if (index == current_order[i])
			{
				isContained = 1;
				previous = i - 1;
			}
		}
		fprintf(stderr, "begin(%d)\n", index);
		if (index != current_order[0] && isContained == 1)
		{
			if (index/1000 == 2 && current_order[previous]/1000 == 1)
			{
				trigger_wait(trigger2);
			} else if (index/1000 == 1 && current_order[previous]/1000 == 2)
			{
				trigger_wait(trigger1);
			}
		}
	}
}

void
inst_end(int index)
{
	if (phase == SCHEDULER)
	{
		int isContained = 0;
		int next;
		for (int i = 0; i < current_length; ++i)
		{
			if (index == current_order[i])
			{
				isContained = 1;
				next = i + 1;
			}
		}
		fprintf(stderr, "end(%d)\n", index);
		if (index != current_order[current_length-1] && isContained == 1)
		{
			if (index/1000 == 1 && current_order[next]/1000 == 2)
			{
				trigger_signal(trigger2);
			} else if (index/1000 == 2 && current_order[next]/1000 == 1)
			{
				trigger_signal(trigger1);
			}
		}
	}
}

void create_exec_order(int which_idiom)
{
	// clean file
	if ((fp = fopen(FILENAME, "w")) == NULL)
	{
		perror("fopen");
		exit(1);
	}

	if (fclose(fp) == EOF)
	{
		perror("fclose");
		exit(1);
	}

	if ((fp = fopen(FILENAME, "a")) == NULL)
	{
		perror("fopen");
		exit(1);
	}

	switch(which_idiom) {
		case 1:
			for (int i = 0; i < thd1_index; ++i)
			{
				for (int j = 0; j < thd2_index; ++j)
				{
					if (thd1[i].accessed_mem_addr == thd2[j].accessed_mem_addr && thd1[i].type == VAR)
					{
						if ((thd1[i].mode == MODE_WRITE && thd2[j].mode == MODE_READ)
							|| (thd1[i].mode == MODE_READ && thd2[j].mode == MODE_WRITE))
						{
							if (thd1[i].instruction_id == thd1_index
								&& thd2[j].instruction_id == 1)
							{
								size[num] = 2;
								exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
								exec_order[num][0] = thd1[i].index;
								exec_order[num][1] = thd2[j].index;
								fprintf(fp, "%d, %d\n", exec_order[num][0], exec_order[num][1]);
							 	num++;
							} else if (thd1[i].instruction_id == thd1_index
								&& thd2[j].instruction_id != 1)
							{
								size[num] = 3;
								exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
								exec_order[num][0] = thd2[j].index - 1;
								exec_order[num][1] = thd1[i].index;
								exec_order[num][2] = thd2[j].index;
								fprintf(fp, "%d, %d, %d\n", 
									exec_order[num][0], exec_order[num][1], exec_order[num][2]);
							 	num++;
							} else if (thd2[j].instruction_id == 1
								&& thd1[i].instruction_id != thd1_index)
							{
								size[num] = 3;
								exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
								exec_order[num][0] = thd1[i].index;
								exec_order[num][1] = thd2[j].index;
								exec_order[num][2] = thd1[i].index + 1;
									fprintf(fp, "%d, %d, %d\n", 
								exec_order[num][0], exec_order[num][1], exec_order[num][2]);
			 					num++;
							} else
							{
								size[num] = 4;
								exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
								exec_order[num][0] = thd2[j].index - 1;
								exec_order[num][1] = thd1[i].index;
								exec_order[num][2] = thd2[j].index;
								exec_order[num][3] = thd1[i].index + 1;
								fprintf(fp, "%d, %d, %d, %d\n", 
									exec_order[num][0], exec_order[num][1], 
									exec_order[num][2], exec_order[num][3]);
			 					num++;
								size[num] = 4;
								exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
								exec_order[num][0] = thd1[i].index - 1;
								exec_order[num][1] = thd2[j].index;
								exec_order[num][2] = thd1[i].index;
								exec_order[num][3] = thd2[j].index + 1;	
								fprintf(fp, "%d, %d, %d, %d\n", 
									exec_order[num][0], exec_order[num][1], 
									exec_order[num][2], exec_order[num][3]);
			 					num++;	 					
							}
						} 
						// if ((thd1[i].mode == MODE_READ && thd2[j].mode == MODE_WRITE)
						// 	|| (thd1[i].mode == MODE_WRITE && thd2[j].mode == MODE_READ))
						// {
						// 	if (thd2[j].instruction_id == thd2_index
						// 		&& thd1[i].instruction_id == 1)
						// 	{
						// 		size[num] = 2;
						// 		exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
						// 		exec_order[num][0] = thd2[j].index;
						// 		exec_order[num][1] = thd1[i].index;
						// 		fprintf(fp, "%d, %d\n", exec_order[num][0], exec_order[num][1]);
				 	// 				num++;
						// 	} else if (thd2[j].instruction_id == thd2_index
						// 		&& thd1[i].instruction_id != 1)
						// 	{
						// 		size[num] = 3;
						// 		exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
						// 		exec_order[num][0] = thd1[i].index - 1;
						// 		exec_order[num][1] = thd2[j].index;
						// 		exec_order[num][2] = thd1[i].index;
						// 		fprintf(fp, "%d, %d, %d\n", 
						// 			exec_order[num][0], exec_order[num][1], exec_order[num][2]);
			 		// 			num++;
						// 	} else if (thd1[i].instruction_id == 1
						// 		&& thd2[j].instruction_id != thd2_index)
						// 	{
						// 		size[num] = 3;
						// 		exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
						// 		exec_order[num][0] = thd2[j].index;
						// 		exec_order[num][1] = thd1[i].index;
						// 		exec_order[num][2] = thd2[j].index + 1;
						// 		fprintf(fp, "%d, %d, %d\n", 
						// 			exec_order[num][0], exec_order[num][1], exec_order[num][2]);
			 		// 			num++;
						// 	} else
						// 	{
						// 		size[num] = 4;
						// 		exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
						// 		exec_order[num][0] = thd1[i].index - 1;
						// 		exec_order[num][1] = thd2[j].index;
						// 		exec_order[num][2] = thd1[i].index;
						// 		exec_order[num][3] = thd2[j].index + 1;
						// 		fprintf(fp, "%d, %d, %d, %d\n", 
						// 			exec_order[num][0], exec_order[num][1], 
						// 			exec_order[num][2], exec_order[num][3]);
			 		// 			num++;
						// 	}
						// }
						num_of_exec_order += 2;
					}
				}
			}

			// for (int i = 0; i < thd1_index; ++i)
			// {
			// 	for (int j = 0; j < thd2_index; ++j)
			// 	{
			// 		if (thd1[i].accessed_mem_addr == thd2[j].accessed_mem_addr && thd1[i].type == VAR)
			// 		{
			// 			if (thd1[i].instruction_id == 1 && thd2[j].instruction_id == thd2_index)
			// 			{
			// 				if (thd1[i].lock != thd1[i+1].lock && thd2[j].lock != thd2[j-1].lock)
			// 				{
			// 					size[num] = 4;
			// 					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 					exec_order[num][0] = thd2[j].index - 1;
			// 					exec_order[num][1] = thd1[i].index;
			// 					exec_order[num][2] = thd2[j].index;
			// 					exec_order[num][3] = thd1[i].index + 1;
			// 					fprintf(fp, "%d, %d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2], exec_order[num][3]);
			// 					num++;
			// 				}
			// 				size[num] = 2;
			// 				exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 				exec_order[num][0] = thd2[j].index;
			// 				exec_order[num][1] = thd1[i].index;
			// 				fprintf(fp, "%d, %d\n", exec_order[num][0], exec_order[num][1]);
			// 				num++;
			// 			} else if (thd1[i].instruction_id == thd1_index && thd2[j].instruction_id == 1)
			// 			{
			// 				size[num] = 4;
			// 				exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 				exec_order[num][0] = thd1[i].index - 1;
			// 				exec_order[num][1] = thd2[j].index;
			// 				exec_order[num][2] = thd1[i].index;
			// 				exec_order[num][3] = thd2[j].index + 1;
			// 				fprintf(fp, "%d, %d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2], exec_order[num][3]);
			// 				num++;
			// 				size[num] = 2;
			// 				exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 				exec_order[num][0] = thd1[i].index;
			// 				exec_order[num][1] = thd2[j].index;
			// 				fprintf(fp, "%d, %d\n", exec_order[num][0], exec_order[num][1]);
			// 				num++;
			// 			} else if (thd1[i].instruction_id == 1 && thd2[j].instruction_id == 1)
			// 			{
			// 				size[num] = 3;
			// 				exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 				exec_order[num][0] = thd1[i].index;
			// 				exec_order[num][1] = thd2[j].index;
			// 				exec_order[num][2] = thd1[i].index + 1;
			// 				fprintf(fp, "%d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2]);
			// 				num++;
			// 				size[num] = 3;
			// 				exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 				exec_order[num][0] = thd2[j].index;
			// 				exec_order[num][1] = thd1[i].index;
			// 				exec_order[num][2] = thd2[j].index + 1;
			// 				fprintf(fp, "%d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2]);
			// 				num++;
			// 			} else if (thd1[i].instruction_id == thd1_index && thd2[j].instruction_id == thd2_index)
			// 			{
			// 				size[num] = 3;
			// 				exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 				exec_order[num][0] = thd1[i].index - 1;
			// 				exec_order[num][1] = thd2[j].index;
			// 				exec_order[num][2] = thd1[i].index;
			// 				fprintf(fp, "%d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2]);
			// 				num++;
			// 				size[num] = 3;
			// 				exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 				exec_order[num][0] = thd2[j].index - 1;
			// 				exec_order[num][1] = thd1[i].index;
			// 				exec_order[num][2] = thd2[j].index;
			// 				fprintf(fp, "%d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2]);
			// 				num++;
			// 			} else if (thd1[i].instruction_id == 1 && thd2[j].instruction_id != 1 && thd2[j].instruction_id != thd2_index)
			// 			{
			// 				size[num] = 4;
			// 				exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 				exec_order[num][0] = thd2[j].index - 1;
			// 				exec_order[num][1] = thd1[i].index;
			// 				exec_order[num][2] = thd2[j].index;
			// 				exec_order[num][3] = thd1[i].index + 1;
			// 				fprintf(fp, "%d, %d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2], exec_order[num][3]);
			// 				num++;
			// 				size[num] = 3;
			// 				exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 				exec_order[num][0] = thd2[j].index;
			// 				exec_order[num][1] = thd1[i].index;
			// 				exec_order[num][2] = thd2[j].index + 1;
			// 				fprintf(fp, "%d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2]);
			// 				num++;
			// 			} else if (thd2[j].instruction_id == 1 && thd1[i].instruction_id != 1 && thd1[i].instruction_id != thd1_index)
			// 			{
			// 				size[num] = 4;
			// 				exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 				exec_order[num][0] = thd1[i].index - 1;
			// 				exec_order[num][1] = thd2[j].index;
			// 				exec_order[num][2] = thd1[i].index;
			// 				exec_order[num][3] = thd2[j].index + 1;
			// 				fprintf(fp, "%d, %d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2], exec_order[num][3]);
			// 				num++;
			// 				size[num] = 3;
			// 				exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 				exec_order[num][0] = thd1[i].index;
			// 				exec_order[num][1] = thd2[j].index;
			// 				exec_order[num][2] = thd1[i].index + 1;
			// 				fprintf(fp, "%d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2]);
			// 				num++;
			// 			} else if (thd1[i].instruction_id == thd1_index && thd2[j].instruction_id != 1 && thd2[j].instruction_id != thd2_index)
			// 			{
			// 				size[num] = 4;
			// 				exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 				exec_order[num][0] = thd1[i].index - 1;
			// 				exec_order[num][1] = thd2[j].index;
			// 				exec_order[num][2] = thd1[i].index;
			// 				exec_order[num][3] = thd2[j].index + 1;
			// 				fprintf(fp, "%d, %d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2], exec_order[num][3]);
			// 				num++;
			// 				size[num] = 3;
			// 				exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 				exec_order[num][0] = thd2[j].index - 1;
			// 				exec_order[num][1] = thd1[i].index;
			// 				exec_order[num][2] = thd2[j].index;
			// 				fprintf(fp, "%d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2]);
			// 				num++;
			// 			} else if (thd2[j].instruction_id == thd2_index && thd1[i].index != 1 && thd1[i].index != thd1_index)
			// 			{
			// 				if (thd2[j].lock == thd1[i].lock && thd1[i].last_in_mutex != 1)
			// 				{
			// 					// do nothing
			// 				} else
			// 				{
			// 					size[num] = 4;
			// 					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 					exec_order[num][0] = thd2[j].index - 1;
			// 					exec_order[num][1] = thd1[i].index;
			// 					exec_order[num][2] = thd2[j].index;
			// 					exec_order[num][3] = thd1[i].index + 1;
			// 					fprintf(fp, "%d, %d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2], exec_order[num][3]);
			// 					num++;
			// 				}
			// 				size[num] = 3;
			// 				exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 				exec_order[num][0] = thd1[i].index - 1;
			// 				exec_order[num][1] = thd2[j].index;
			// 				exec_order[num][2] = thd1[i].index;
			// 				fprintf(fp, "%d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2]);
			// 				num++;
			// 			} else
			// 			{
			// 				if (thd1[i].lock != NULL && thd1[i].lock == thd2[j].lock)
			// 				{
			// 					// do nothing
			// 				}
			// 				else
			// 				{
			// 					size[num] = 4;
			// 					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 					if (thd2[j-1].type == SYNC && thd2[j].first_in_mutex == 1)
			// 					{
			// 						exec_order[num][0] = thd2[j-2].index;
			// 					} else 
			// 					{
			// 						exec_order[num][0] = thd2[j].index - 1;
			// 					}
			// 					exec_order[num][1] = thd1[i].index;
			// 					if (thd1[i].lock == thd2[j].lock && thd1[i].lock != NULL && thd2[j].last_in_mutex == 1)
			// 					{
			// 						exec_order[num][2] = thd2[j].index + 1;
			// 					} else 
			// 					{
			// 						exec_order[num][2] = thd2[j].index;
			// 					}
			// 					exec_order[num][0] = thd2[j].index - 1;
			// 					exec_order[num][1] = thd1[i].index;
			// 					exec_order[num][2] = thd2[j].index;
			// 					exec_order[num][3] = thd1[i].index + 1;
			// 					fprintf(fp, "%d, %d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2], exec_order[num][3]);
			// 					num++;
			// 					size[num] = 4;
			// 					exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
			// 					exec_order[num][0] = thd1[i].index - 1;
			// 					exec_order[num][1] = thd2[j].index;
			// 					exec_order[num][2] = thd1[i].index;
			// 					exec_order[num][3] = thd2[j].index + 1;
			// 					fprintf(fp, "%d, %d, %d, %d\n", exec_order[num][0], exec_order[num][1], exec_order[num][2], exec_order[num][3]);
			// 					num++;
			// 				}
			// 			}
			// 			num_of_exec_order += 2;
			// 		}
			// 	}
			// }
			break;

		case 2:
			for (int m = 0; m < thd1_index; ++m)
			{
				for (int i = 0; i < thd2_index; ++i)
				{
					for (int n = m + 1; n < thd1_index; ++n)
					{
						if (thd1[m].accessed_mem_addr == thd2[i].accessed_mem_addr && 
						thd1[n].accessed_mem_addr == thd2[i].accessed_mem_addr)
						{
							if (thd1[m].mode == MODE_WRITE
								&& thd2[i].mode == MODE_WRITE
								&& thd1[n].mode == MODE_READ)
							{
								int same_access = 0;
								for (int x = m+1; x < n; ++x)
								{
									if (thd1[x].accessed_mem_addr == thd1[m].accessed_mem_addr)
									{
										same_access = 1;
									}
								}
								if (same_access == 0)
								{
									if (thd2[i].instruction_id == 1)
									{
										size[num] = 4;
										exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
										exec_order[num][0] = thd1[m].index;
										exec_order[num][1] = thd2[i].index;
										exec_order[num][2] = thd1[n].index;
										exec_order[num][3] = thd2[i].index + 1;
										fprintf(fp, "%d, %d, %d, %d\n", 
											exec_order[num][0], exec_order[num][1], 
											exec_order[num][2], exec_order[num][3]);
			 							num++;
									} else if (thd2[i].instruction_id == thd2_index)
									{
										size[num] = 4;
										exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
										exec_order[num][0] = thd2[i].index - 1;
										exec_order[num][1] = thd1[m].index;
										exec_order[num][2] = thd2[i].index;
										exec_order[num][3] = thd1[n].index;
										fprintf(fp, "%d, %d, %d, %d\n", 
											exec_order[num][0], exec_order[num][1], 
											exec_order[num][2], exec_order[num][3]);
			 							num++;
									} else 
									{
										size[num] = 5;
										exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
										exec_order[num][0] = thd2[i].index - 1;
										exec_order[num][1] = thd1[m].index;
										exec_order[num][2] = thd2[i].index;
										exec_order[num][3] = thd1[n].index;
										exec_order[num][4] = thd2[i].index + 1;
										fprintf(fp, "%d, %d, %d, %d, %d\n", 
											exec_order[num][0], exec_order[num][1], 
											exec_order[num][2], exec_order[num][3], exec_order[num][4]);
			 							num++;
									}
									num_of_exec_order += 1;
								}
							}
						}
					}
				}
			}

			for (int m = 0; m < thd2_index; ++m)
			{
				for (int i = 0; i < thd1_index; ++i)
				{
					for (int n = m + 1; n < thd2_index; ++n)
					{
						if (thd2[m].accessed_mem_addr == thd1[i].accessed_mem_addr && 
							thd2[n].accessed_mem_addr == thd1[i].accessed_mem_addr)
						{
							if (thd2[m].mode == MODE_WRITE
								&& thd1[i].mode == MODE_WRITE
								&& thd2[n].mode == MODE_READ)
							{
								int same_access = 0;
								for (int x = m+1; x < n; ++x)
								{
									if (thd2[x].accessed_mem_addr == thd2[m].accessed_mem_addr)
									{
										same_access = 1;
									}
								}
								if (same_access == 0)
								{
									if (thd1[i].instruction_id == 1)
									{
										size[num] = 4;
										exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
										exec_order[num][0] = thd2[m].index;
										exec_order[num][1] = thd1[i].index;
										exec_order[num][2] = thd2[n].index;
										exec_order[num][3] = thd1[i].index + 1;
										fprintf(fp, "%d, %d, %d, %d\n", 
											exec_order[num][0], exec_order[num][1], 
											exec_order[num][2], exec_order[num][3]);
			 							num++;
									} else if (thd1[i].instruction_id == thd1_index)
									{
										size[num] = 4;
										exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
										exec_order[num][0] = thd1[i].index - 1;
										exec_order[num][1] = thd2[m].index;
										exec_order[num][2] = thd1[i].index;
										exec_order[num][3] = thd2[n].index;
										fprintf(fp, "%d, %d, %d, %d\n", 
											exec_order[num][0], exec_order[num][1], 
											exec_order[num][2], exec_order[num][3]);
			 							num++;
									} else 
									{
										size[num] = 5;
										exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
										exec_order[num][0] = thd1[i].index - 1;
										exec_order[num][1] = thd2[m].index;
										exec_order[num][2] = thd1[i].index;
										exec_order[num][3] = thd2[n].index;
										exec_order[num][4] = thd1[i].index + 1;
										fprintf(fp, "%d, %d, %d, %d, %d\n", 
											exec_order[num][0], exec_order[num][1], 
											exec_order[num][2], exec_order[num][3], exec_order[num][4]);
			 							num++;
									}
									num_of_exec_order += 1;
								}
							}
						}
					}
				}
			}

			break;

		case 3:
			for (int i = 0; i < thd1_index; ++i)
			{
				for (int m = 0; m < thd2_index; ++m)
				{
					for (int j = i + 1; j < thd1_index; ++j)
					{
						for (int n = m + 1; n < thd2_index; ++n)
						{
							if (thd1[i].accessed_mem_addr == thd1[j].accessed_mem_addr 
							&& thd1[i].accessed_mem_addr == thd2[m].accessed_mem_addr
							&& thd1[i].accessed_mem_addr == thd2[n].accessed_mem_addr)
							{
								if (thd1[i].mode == MODE_WRITE
									&& thd2[n].mode == MODE_WRITE
									&& thd1[j].mode == MODE_READ)
								{
									int same_access = 0;
									for (int x = i + 1; x < j; ++x)
									{
										if (thd1[x].accessed_mem_addr == thd1[i].accessed_mem_addr)
										{
											same_access = 1;
										}
									}
									if (same_access == 0)
									{
										if (thd2[m].instruction_id == 1
											&& thd2[n].instruction_id == thd2_index)
										{
											size[num] = 4;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd1[i].index;
											exec_order[num][1] = thd2[m].index;
											exec_order[num][2] = thd2[n].index;
											exec_order[num][3] = thd1[j].index;
											fprintf(fp, "%d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3]);
			 								num++;
										} else if (thd2[m].instruction_id == 1
											&& thd2[n].instruction_id != thd2_index)
										{
											size[num] = 5;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd1[i].index;
											exec_order[num][1] = thd2[m].index;
											exec_order[num][2] = thd2[n].index;
											exec_order[num][3] = thd1[j].index;
											exec_order[num][4] = thd2[n].index + 1;
											fprintf(fp, "%d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], exec_order[num][4]);
			 								num++;
										} else if (thd2[m].instruction_id != 1
											&& thd2[m].instruction_id != thd2_index
											&& thd2[n].instruction_id == thd2_index)
										{
											size[num] = 5;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd2[m].index - 1;
											exec_order[num][1] = thd1[i].index;
											exec_order[num][2] = thd2[m].index;
											exec_order[num][3] = thd2[n].index;
											exec_order[num][4] = thd1[j].index;
											fprintf(fp, "%d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], exec_order[num][4]);
			 								num++;
										} else if (thd2[m].instruction_id != 1
											&& thd2[m].instruction_id != thd2_index
											&& thd2[n].instruction_id != thd2_index)
										{
											size[num] = 6;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd2[m].index - 1;
											exec_order[num][1] = thd1[i].index;
											exec_order[num][2] = thd2[m].index;
											exec_order[num][3] = thd2[n].index;
											exec_order[num][4] = thd1[j].index;
											exec_order[num][5] = thd2[n].index + 1;
											fprintf(fp, "%d, %d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], 
												exec_order[num][4], exec_order[num][5]);
			 								num++;
										}
										num_of_exec_order += 1;
									}
								}
							}
						}
					}
				}
			}

			for (int i = 0; i < thd2_index; ++i)
			{
				for (int m = 0; m < thd1_index; ++m)
				{
					for (int j = i + 1; j < thd2_index; ++j)
					{
						for (int n = m + 1; n < thd1_index; ++n)
						{
							if (thd2[i].accessed_mem_addr == thd2[j].accessed_mem_addr 
							&& thd2[i].accessed_mem_addr == thd1[m].accessed_mem_addr
							&& thd2[i].accessed_mem_addr == thd1[n].accessed_mem_addr)
							{
								if (thd2[i].mode == MODE_WRITE
									&& thd1[n].mode == MODE_WRITE
									&& thd2[j].mode == MODE_READ)
								{
									int same_access = 0;
									for (int x = i + 1; x < j; ++x)
									{
										if (thd2[x].accessed_mem_addr == thd2[i].accessed_mem_addr)
										{
											same_access = 1;
										}
									}
									if (same_access == 0)
									{
										if (thd1[m].instruction_id == 1
											&& thd1[n].instruction_id == thd1_index)
										{
											size[num] = 4;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd2[i].index;
											exec_order[num][1] = thd1[m].index;
											exec_order[num][2] = thd1[n].index;
											exec_order[num][3] = thd2[j].index;
											fprintf(fp, "%d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3]);
			 								num++;
										} else if (thd1[m].instruction_id == 1
											&& thd1[n].instruction_id != thd1_index)
										{
											size[num] = 5;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd2[i].index;
											exec_order[num][1] = thd1[m].index;
											exec_order[num][2] = thd1[n].index;
											exec_order[num][3] = thd2[j].index;
											exec_order[num][4] = thd1[n].index + 1;
											fprintf(fp, "%d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], exec_order[num][4]);
			 								num++;
										} else if (thd1[m].instruction_id != 1
											&& thd1[m].instruction_id != thd1_index
											&& thd1[n].instruction_id == thd1_index)
										{
											size[num] = 5;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd1[m].index - 1;
											exec_order[num][1] = thd2[i].index;
											exec_order[num][2] = thd1[m].index;
											exec_order[num][3] = thd1[n].index;
											exec_order[num][4] = thd2[j].index;
											fprintf(fp, "%d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], exec_order[num][4]);
			 								num++;
										} else if (thd1[m].instruction_id != 1
											&& thd1[m].instruction_id != thd1_index
											&& thd1[n].instruction_id != thd1_index)
										{
											size[num] = 6;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd1[m].index - 1;
											exec_order[num][1] = thd2[i].index;
											exec_order[num][2] = thd1[m].index;
											exec_order[num][3] = thd1[n].index;
											exec_order[num][4] = thd2[j].index;
											exec_order[num][5] = thd1[n].index + 1;
											fprintf(fp, "%d, %d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], 
												exec_order[num][4], exec_order[num][5]);
			 								num++;
										}
										num_of_exec_order += 1;
									}
								}
							}
						}
					}
				}
			}
			break;
		case 4:
			for (int i = 0; i < thd1_index; ++i)
			{
				for (int m = 0; m < thd2_index; ++m)
				{
					for (int j = i + 1; j < thd1_index; ++j)
					{
						for (int n = m + 1; n < thd2_index; ++n)
						{
							if (thd1[i].accessed_mem_addr == thd2[m].accessed_mem_addr 
							&& thd1[j].accessed_mem_addr == thd2[n].accessed_mem_addr)
							{
								if (thd1[i].mode == MODE_WRITE
									&& thd2[m].mode == MODE_READ
									&& thd2[n].mode == MODE_WRITE
									&& thd1[j].mode == MODE_READ)
								{
									int same_access = 0;
									for (int x = i + 1; x < j; ++x)
									{
										if (thd1[x].accessed_mem_addr == thd1[i].accessed_mem_addr
											|| thd1[x].accessed_mem_addr == thd1[j].accessed_mem_addr)
										{
											same_access = 1;
										}
									}
									if (same_access == 0)
									{
										if (thd2[m].instruction_id == 1
											&& thd2[n].instruction_id == thd2_index)
										{
											size[num] = 4;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd1[i].index;
											exec_order[num][1] = thd2[m].index;
											exec_order[num][2] = thd2[n].index;
											exec_order[num][3] = thd1[j].index;
											fprintf(fp, "%d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3]);
			 								num++;
										} else if (thd2[m].instruction_id == 1
											&& thd2[n].instruction_id != thd2_index)
										{
											size[num] = 5;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd1[i].index;
											exec_order[num][1] = thd2[m].index;
											exec_order[num][2] = thd2[n].index;
											exec_order[num][3] = thd1[j].index;
											exec_order[num][4] = thd2[n].index + 1;
											fprintf(fp, "%d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], exec_order[num][4]);
			 								num++;
										} else if (thd2[m].instruction_id != 1
											&& thd2[m].instruction_id != thd2_index
											&& thd2[n].instruction_id == thd2_index)
										{
											size[num] = 5;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd2[m].index - 1;
											exec_order[num][1] = thd1[i].index;
											exec_order[num][2] = thd2[m].index;
											exec_order[num][3] = thd2[n].index;
											exec_order[num][4] = thd1[j].index;
											fprintf(fp, "%d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], exec_order[num][4]);
			 								num++;
										} else if (thd2[m].instruction_id != 1
											&& thd2[m].instruction_id != thd2_index
											&& thd2[n].instruction_id != thd2_index)
										{
											size[num] = 6;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd2[m].index - 1;
											exec_order[num][1] = thd1[i].index;
											exec_order[num][2] = thd2[m].index;
											exec_order[num][3] = thd2[n].index;
											exec_order[num][4] = thd1[j].index;
											exec_order[num][5] = thd2[n].index + 1;
											fprintf(fp, "%d, %d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], 
												exec_order[num][4], exec_order[num][5]);
			 								num++;
										}
										num_of_exec_order += 1;
									}
								}
							}
						}
					}
				}
			}

			for (int i = 0; i < thd2_index; ++i)
			{
				for (int m = 0; m < thd1_index; ++m)
				{
					for (int j = i + 1; j < thd2_index; ++j)
					{
						for (int n = m + 1; n < thd1_index; ++n)
						{
							if (thd2[i].accessed_mem_addr == thd1[m].accessed_mem_addr 
							&& thd2[j].accessed_mem_addr == thd1[n].accessed_mem_addr)
							{
								if (thd2[i].mode == MODE_WRITE
									&& thd1[m].mode == MODE_READ
									&& thd1[n].mode == MODE_WRITE
									&& thd2[j].mode == MODE_READ)
								{
									int same_access = 0;
									for (int x = i + 1; x < j; ++x)
									{
										if (thd2[x].accessed_mem_addr == thd2[i].accessed_mem_addr
											|| thd2[x].accessed_mem_addr == thd2[j].accessed_mem_addr)
										{
											same_access = 1;
										}
									}
									if (same_access == 0)
									{
										if (thd1[m].instruction_id == 1
											&& thd1[n].instruction_id == thd1_index)
										{
											size[num] = 4;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd2[i].index;
											exec_order[num][1] = thd1[m].index;
											exec_order[num][2] = thd1[n].index;
											exec_order[num][3] = thd2[j].index;
											fprintf(fp, "%d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3]);
			 								num++;
										} else if (thd1[m].instruction_id == 1
											&& thd1[n].instruction_id != thd1_index)
										{
											size[num] = 5;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd2[i].index;
											exec_order[num][1] = thd1[m].index;
											exec_order[num][2] = thd1[n].index;
											exec_order[num][3] = thd2[j].index;
											exec_order[num][4] = thd1[n].index + 1;
											fprintf(fp, "%d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], exec_order[num][4]);
			 								num++;
										} else if (thd1[m].instruction_id != 1
											&& thd1[m].instruction_id != thd1_index
											&& thd1[n].instruction_id == thd1_index)
										{
											size[num] = 5;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd1[m].index - 1;
											exec_order[num][1] = thd2[i].index;
											exec_order[num][2] = thd1[m].index;
											exec_order[num][3] = thd1[n].index;
											exec_order[num][4] = thd2[j].index;
											fprintf(fp, "%d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], exec_order[num][4]);
			 								num++;
										} else if (thd1[m].instruction_id != 1
											&& thd1[m].instruction_id != thd1_index
											&& thd1[n].instruction_id != thd1_index)
										{
											size[num] = 6;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd1[m].index - 1;
											exec_order[num][1] = thd2[i].index;
											exec_order[num][2] = thd1[m].index;
											exec_order[num][3] = thd1[n].index;
											exec_order[num][4] = thd2[j].index;
											exec_order[num][5] = thd1[n].index + 1;
											fprintf(fp, "%d, %d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], 
												exec_order[num][4], exec_order[num][5]);
			 								num++;
										}
										num_of_exec_order += 1;
									}
								}
							}
						}
					}
				}
			}

			break;
		case 5:
			for (int i = 0; i < thd1_index; ++i)
			{
				for (int m = 0; m < thd2_index; ++m)
				{
					for (int j = i + 1; j < thd1_index; ++j)
					{
						for (int n = m + 1; n < thd2_index; ++n)
						{
							if (thd1[i].accessed_mem_addr == thd2[m].accessed_mem_addr 
							&& thd1[j].accessed_mem_addr == thd2[n].accessed_mem_addr)
							{
								if (thd1[i].mode == MODE_WRITE
									&& thd2[m].mode == MODE_READ
									&& thd2[n].mode == MODE_WRITE
									&& thd1[j].mode == MODE_READ)
								{
									int same_access = 0;
									for (int x = i + 1; x < j; ++x)
									{
										if (thd1[x].accessed_mem_addr == thd1[i].accessed_mem_addr
											|| thd1[x].accessed_mem_addr == thd1[j].accessed_mem_addr)
										{
											same_access = 1;
										}
									}
									if (same_access == 0)
									{
										if (thd2[m].instruction_id == 1
											&& thd2[n].instruction_id == thd2_index)
										{
											size[num] = 4;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd1[i].index;
											exec_order[num][1] = thd2[m].index;
											exec_order[num][2] = thd2[n].index;
											exec_order[num][3] = thd1[j].index;
											fprintf(fp, "%d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3]);
			 								num++;
										} else if (thd2[m].instruction_id == 1
											&& thd2[n].instruction_id != thd2_index)
										{
											size[num] = 5;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd1[i].index;
											exec_order[num][1] = thd2[m].index;
											exec_order[num][2] = thd2[n].index;
											exec_order[num][3] = thd1[j].index;
											exec_order[num][4] = thd2[n].index + 1;
											fprintf(fp, "%d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], exec_order[num][4]);
			 								num++;
										} else if (thd2[m].instruction_id != 1
											&& thd2[m].instruction_id != thd2_index
											&& thd2[n].instruction_id == thd2_index)
										{
											size[num] = 5;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd2[m].index - 1;
											exec_order[num][1] = thd1[i].index;
											exec_order[num][2] = thd2[m].index;
											exec_order[num][3] = thd2[n].index;
											exec_order[num][4] = thd1[j].index;
											fprintf(fp, "%d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], exec_order[num][4]);
			 								num++;
										} else if (thd2[m].instruction_id != 1
											&& thd2[m].instruction_id != thd2_index
											&& thd2[n].instruction_id != thd2_index)
										{
											size[num] = 6;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd2[m].index - 1;
											exec_order[num][1] = thd1[i].index;
											exec_order[num][2] = thd2[m].index;
											exec_order[num][3] = thd2[n].index;
											exec_order[num][4] = thd1[j].index;
											exec_order[num][5] = thd2[n].index + 1;
											fprintf(fp, "%d, %d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], 
												exec_order[num][4], exec_order[num][5]);
			 								num++;
										}
										num_of_exec_order += 1;
									}
								}
							}
						}
					}
				}
			}

			for (int i = 0; i < thd2_index; ++i)
			{
				for (int m = 0; m < thd1_index; ++m)
				{
					for (int j = i + 1; j < thd2_index; ++j)
					{
						for (int n = m + 1; n < thd1_index; ++n)
						{
							if (thd2[i].accessed_mem_addr == thd1[m].accessed_mem_addr 
							&& thd2[j].accessed_mem_addr == thd1[n].accessed_mem_addr)
							{
								if (thd2[i].mode == MODE_WRITE
									&& thd1[m].mode == MODE_READ
									&& thd1[n].mode == MODE_WRITE
									&& thd2[j].mode == MODE_READ)
								{
									int same_access = 0;
									for (int x = i + 1; x < j; ++x)
									{
										if (thd2[x].accessed_mem_addr == thd2[i].accessed_mem_addr
											|| thd2[x].accessed_mem_addr == thd2[j].accessed_mem_addr)
										{
											same_access = 1;
										}
									}
									if (same_access == 0)
									{
										if (thd1[m].instruction_id == 1
											&& thd1[n].instruction_id == thd1_index)
										{
											size[num] = 4;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd2[i].index;
											exec_order[num][1] = thd1[m].index;
											exec_order[num][2] = thd1[n].index;
											exec_order[num][3] = thd2[j].index;
											fprintf(fp, "%d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3]);
			 								num++;
										} else if (thd1[m].instruction_id == 1
											&& thd1[n].instruction_id != thd1_index)
										{
											size[num] = 5;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd2[i].index;
											exec_order[num][1] = thd1[m].index;
											exec_order[num][2] = thd1[n].index;
											exec_order[num][3] = thd2[j].index;
											exec_order[num][4] = thd1[n].index + 1;
											fprintf(fp, "%d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], exec_order[num][4]);
			 								num++;
										} else if (thd1[m].instruction_id != 1
											&& thd1[m].instruction_id != thd1_index
											&& thd1[n].instruction_id == thd1_index)
										{
											size[num] = 5;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd1[m].index - 1;
											exec_order[num][1] = thd2[i].index;
											exec_order[num][2] = thd1[m].index;
											exec_order[num][3] = thd1[n].index;
											exec_order[num][4] = thd2[j].index;
											fprintf(fp, "%d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], exec_order[num][4]);
			 								num++;
										} else if (thd1[m].instruction_id != 1
											&& thd1[m].instruction_id != thd1_index
											&& thd1[n].instruction_id != thd1_index)
										{
											size[num] = 6;
											exec_order[num] = (int *)malloc(sizeof(int) * size[num]);
											exec_order[num][0] = thd1[m].index - 1;
											exec_order[num][1] = thd2[i].index;
											exec_order[num][2] = thd1[m].index;
											exec_order[num][3] = thd1[n].index;
											exec_order[num][4] = thd2[j].index;
											exec_order[num][5] = thd1[n].index + 1;
											fprintf(fp, "%d, %d, %d, %d, %d, %d\n", 
												exec_order[num][0], exec_order[num][1], 
												exec_order[num][2], exec_order[num][3], 
												exec_order[num][4], exec_order[num][5]);
			 								num++;
										}
										num_of_exec_order += 1;
									}
								}
							}
						}
					}
				}
			}
			break;
	}

	

	if (fclose(fp) == EOF)
	{
		perror("fclose");
		exit(1);
	}
}

void assign_order()
{
	current_order = malloc(sizeof(int) * size[which]);
	for (int i = 0; i < size[which]; ++i)
	{
		current_order[i] = exec_order[which][i];
	}
	current_length = size[which];
	which++;
	free(current_order);
}

void test(pthread_t t1, pthread_t t2, void *func1, void *func2)
{	
	for (int i = 0; i < num; ++i)
	{	
		printf("Execution %d:\n", i + 1);
		assign_order();
		inst_initialize();
		pthread_create(&t1, NULL, func1, NULL);
		pthread_create(&t2, NULL, func2, NULL);
		pthread_join(t1, NULL);
		pthread_join(t2, NULL);
		inst_uninitialize();
		printf("\n");
	}
}

void
crash()
{
	fprintf(stderr, "crash()\n");
}
