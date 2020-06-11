// this is profiler.h

#define PROFILER 0
#define SCHEDULER 1

#define FILENAME "exec_order.txt"

enum mode {
    NONE = 0,                   /* reserved */
    MODE_READ,                  /* 1 */
    MODE_WRITE,                 /* 2 */
};

enum type {
	VAR, 						/* shared variable */
	SYNC,						/* synchronization object */
};

struct info{
	int index;
	int type;
	int thread_id;
	int instruction_id;
	int *accessed_mem_addr;
	int mode; // 1 if read, 2 if write
	pthread_mutex_t *lock;
	int first_in_mutex;
	int last_in_mutex;
};

extern void
inst_initialize();

extern void
inst_uninitialize();

extern void
inst_begin(int index, int type, int *accessed, int mode, pthread_mutex_t *lock, int first, int last);

extern void
inst_end(int index);

extern void
activate_profiler();

extern void
activate_scheduler();

extern void
create_exec_order();

extern void
init_exec_order();

extern void
assign_order();

extern void
test(pthread_t t1, pthread_t t2, void *func1, void *func2);

extern void
crash();
