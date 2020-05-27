// This is trigger.h

typedef struct {
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int isSignaled;
} trigger;

struct info{
	int thread_id;
	int instruction_id;
	int *accessed_mem_addr;
	// int is_mutex;
	// pthread_mutex_t lock;
	int mode; // 0 if read, 1 if write
};

extern trigger *
trigger_init();

extern void
trigger_destroy(trigger *t);

extern void
trigger_signal(trigger *t);

extern void
trigger_wait(trigger *t);

extern void
inst_initialize(int *exec_order, int exec_length);

extern void
inst_uninitialize();

extern struct info
inst_begin(int index, int *accessed, int mode);

extern void
inst_end(int index);

extern void
crash();