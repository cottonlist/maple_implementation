// this is profiler.h

#define PROFILER 0
#define SCHEDULER 1

struct info{
	int thread_id;
	int instruction_id;
	int *accessed_mem_addr;
	// int is_mutex;
	// pthread_mutex_t lock;
	int mode; // 0 if read, 1 if write
};

int phase;

extern void
inst_initialize(int *exec_order, int exec_length);

extern void
inst_uninitialize();

extern void
inst_begin(int index, int *accessed, int mode);

extern void
inst_end(int index);

extern void
profiler_activator();

extern void
scheduler_activator();

extern void
test();

extern void
crash();
