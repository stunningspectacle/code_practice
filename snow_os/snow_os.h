#ifndef __snow_os_h__
#define __snow_os_h__

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define SNOW_HZ			100
#define SNOW_THREAD_SCHED_US	1000
#define SNOW_SYS_STACK		512

struct list_t {
	struct list_t *next, *prev;
};

struct snow_mutex_t {
	char		*name;
	void		*thread;
	int		count;
	struct list_t	list; 
};

struct snow_sem_t {
	char		*name;
	int		count;
	struct list_t	list; 
};

struct snow_timer_t {
	char		*name;
	unsigned long long us;
	void(*timer_fn)(void *);
	void		*arg;
	struct list_t	list; 
};

struct snow_thread_t {
	char		*sp;
	char		*name;
	int		flags;
	void(*thread_fn)(void *);
	void		*thread_arg;
	int		stack_size;
	char		*stack;
	struct list_t	list; 
	unsigned long long timeout;
	unsigned int	sched_us;
	struct snow_timer_t wakeup_timer;
};

void snow_start(void(*main)(void *),
		void *main_sp,
		int main_stack_size,
		void *idle_stack);
void *snow_thread_create(char *name, 
		void(*thread_fn)(void *),
		void *arg,
		void *sp,
		int stack_size);
void snow_sched(void);
void snow_timer_init(struct snow_timer_t *timer,
		char *name,
		void(*timer_fn)(void *),
		void *arg);
void snow_timer_add(struct snow_timer_t *timer, unsigned int us);
void snow_timer_del(struct snow_timer_t *timer);
void snow_sem_init(struct snow_sem_t *sem, char *name, int count);
int snow_sem_down_timeout(struct snow_sem_t *sem, unsigned int us);
#define snow_sem_down(sem) do{while(snow_sem_down_timeout(sem, -1) != 0);}while(0)
#define snow_sem_trydown(sem) snow_sem_down_timeout(sem, 0)
void snow_sem_up(struct snow_sem_t *sem);
void snow_mutex_init(struct snow_mutex_t *mutex, char *name);
int snow_mutex_lock_timeout(struct snow_mutex_t *mutex, unsigned int us);
#define snow_mutex_lock(mutex) do{while(snow_mutex_lock_timeout(mutex, -1) != 0);}while(0)
#define snow_mutex_trylock(mutex) snow_mutex_lock_timeout(mutex, 0)
void snow_mutex_unlock(struct snow_mutex_t *mutex);
unsigned long long snow_gettime_us(void);
void snow_usleep(unsigned int us);
void snow_udelay(unsigned int us);
#endif

