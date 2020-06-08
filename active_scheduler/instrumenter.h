// this is profiler.h

#define PROFILER 0
#define SCHEDULER 1

#define FILENAME "exec_order.txt"

enum {
    NONE = 0,                   /* reserved */
    MODE_READ,                  /* 1 */
    MODE_WRITE,                 /* 2 */
};

struct info{
	int thread_id;
	int instruction_id;
	int *accessed_mem_addr;
	// int is_mutex;
	// pthread_mutex_t lock;
	int mode; // 1 if read, 2 if write
};

extern void
inst_initialize(int *exec_order, int exec_length);

extern void
inst_uninitialize();

extern void
inst_begin(int index, int *accessed, int mode);

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
crash();
