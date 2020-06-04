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

