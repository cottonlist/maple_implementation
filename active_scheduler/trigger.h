// This is trigger.h

typedef struct {
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int isSignaled;
} trigger;

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

extern void
inst_begin(int index);

extern void
inst_end(int index);

extern void
crash();