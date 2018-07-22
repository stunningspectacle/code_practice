#include <snow_os.h>
#include <snow_internal.h>

#define FLAGS_RUN_MASK		0xff
#define FLAGS_RUN		(1 << 0)
#define FLAGS_SLEEPING		(1 << 1)
#define FLAGS_WAIT		(1 << 2)

#define SNOW_SYSCALL_DUMMY		0
#define SNOW_SYSCALL_SCHED		1
#define SNOW_SYSCALL_FORKIDLE		2

#define holder_of(member_ptr, holder_type, member_name) \
		__holder_of(member_ptr, holder_type, member_name)
#define __holder_of(ptr, type, member) \
		((type *)((char *)(ptr) - ((int)&((type *)0)->member)))

#define list_foreach(list_each, list_tmp, list_head) \
	for (list_each = (list_head)->next, list_tmp = list_each->next; \
			list_each != (list_head);			\
			list_each = list_tmp, list_tmp = list_each->next)

#define LIST_DEF(name) \
	struct list_t name = {&name, &name};

static LIST_DEF(sched_list);
static LIST_DEF(timer_list);
static int interrupt_disable_depth;
static volatile unsigned long long sys_time_us = 0;
struct snow_thread_t *current, *idle;


static inline void list_init(struct list_t *list)
{
	list->next = list;
	list->prev = list;
}

static inline void list_insert(struct list_t *new_list,
                              struct list_t *prev_list,
                              struct list_t *next_list)
{
	next_list->prev = new_list;
	new_list->next = next_list;
	new_list->prev = prev_list;
	prev_list->next = new_list;
}

static inline void list_remove(struct list_t *list)
{
	struct list_t * prev_list = list->prev;
	struct list_t * next_list = list->next;

        next_list->prev = prev_list;
        prev_list->next = next_list;
	list_init(list);
}

static inline int list_empty(const struct list_t *list)
{
	return list->next == list;
}


static inline void list_fifo_put(struct list_t *new_list, struct list_t *fifo)
{
	list_insert(new_list, fifo->prev, fifo);
}

static inline struct list_t *list_fifo_get(struct list_t *fifo)
{
	struct list_t *list;

	if (list_empty(fifo))
		return NULL;	
	list = fifo->next;
	list_remove(list);
	return list;
}

static inline int thread_is_run(struct snow_thread_t *thread)
{
	return thread->flags & FLAGS_RUN;
}

static inline int thread_is_wait(struct snow_thread_t *thread)
{
	return thread->flags & FLAGS_WAIT;
}

static inline void thread_set_flag(struct snow_thread_t *thread, int flag)
{
	if (flag & FLAGS_RUN_MASK)
		thread->flags = (thread->flags & ~FLAGS_RUN_MASK) | flag;
	else
		thread->flags |= flag;
	
}

static void interrupt_enable(void)
{
	interrupt_disable_depth--;
	if (interrupt_disable_depth < 0)
		arch_print("unbalance interrupt enable\n");
	if (interrupt_disable_depth == 0)
		arch_enable_interrupt();
}

static void interrupt_disable(void)
{
	arch_disable_interrupt();
	interrupt_disable_depth++;
}

unsigned long long snow_gettime_us(void)
{
//	unsigned int us;
	unsigned long long res;

	interrupt_disable();
//	us =  SysTick->VAL/(SystemCoreClock/1000000);
//	res = sys_time_us + us;
	res = sys_time_us;
	interrupt_enable();

	return res;
}

void snow_udelay(unsigned int us)
{
	unsigned long long start = snow_gettime_us();
	while(snow_gettime_us() < start + us)
		;
}

void init_thread(struct snow_thread_t *thread_new)
{
	arch_print("init new thread\n");
	thread_new->thread_fn(thread_new->thread_arg);
	arch_print("thread end ...\n");
	while (1) ;
}

void snow_wakeup(struct snow_thread_t *thread)
{
	interrupt_disable();
	if (!thread_is_run(thread)) {
		thread_set_flag(thread, FLAGS_RUN);
		list_remove(&thread->list);
		list_fifo_put(&thread->list, &sched_list);
	}
	snow_timer_del(&thread->wakeup_timer);
	interrupt_enable();
}

void snow_usleep(unsigned int us)
{
	if (current == idle) {
		arch_print("Error: cannot sleep in idle\n");
		return;
	}
	interrupt_disable();
	thread_set_flag(current, FLAGS_SLEEPING);
	snow_timer_add(&current->wakeup_timer, us);
	interrupt_enable();
	snow_sched();
}

static int wait_get_timeout(struct list_t *wait_list, unsigned int us, int(*get)(void *), void *arg)
{
	unsigned long long timeout, cur_us;
	int need_wakeup = 0;

	if (us == 0) {
		timeout = 0;
		need_wakeup = 0;
	} else if (us == -1) {
		timeout = -1;
		need_wakeup = 0;
	} else {
		timeout = snow_gettime_us() + us;
		need_wakeup = 1;
	}
again:
	interrupt_disable();
	if (get(arg))
		goto success;
	cur_us = snow_gettime_us();
	if (timeout <= cur_us)
		goto fail;
	thread_set_flag(current, FLAGS_WAIT);
	list_fifo_put(&current->list, wait_list);
	if (need_wakeup)
		snow_timer_add(&current->wakeup_timer, timeout - cur_us);
	interrupt_enable();
	snow_sched();
	goto again;
success:
	interrupt_enable();
	return 0;
fail:
	interrupt_enable();
	return -1;
}

static int wait_put(struct list_t *wait_list, int(*put)(void *), void *arg)
{
	struct list_t *list;
	struct snow_thread_t *thread;
	int ret = 0;

	interrupt_disable();
	if (put(arg) && !list_empty(wait_list)) {
		list = list_fifo_get(wait_list);
		thread = holder_of(list, struct snow_thread_t, list);
		snow_wakeup(thread);
		ret = 1;
	}
	interrupt_enable();
	return ret;
}

void snow_mutex_init(struct snow_mutex_t *mutex, char *name)
{
	mutex->name = name;
	mutex->count = 0;
	list_init(&mutex->list);
}

static int mutex_get(struct snow_mutex_t *mutex)
{
	if (mutex->thread == NULL)
		mutex->thread = current;
	if (mutex->thread == current) {
		mutex->count ++;
		return 1;
	}
	return 0;
}

static int mutex_put(struct snow_mutex_t *mutex)
{
	mutex->count--;
	arch_assert((mutex->thread != current || mutex->count < 0), "unbalance mutex\n");
	if (mutex->count == 0) {
		mutex->thread = NULL;
		return 1;
	}
	return 0;
}

int snow_mutex_lock_timeout(struct snow_mutex_t *mutex, unsigned int us)
{
	return wait_get_timeout(&mutex->list, us, (void *)mutex_get, mutex);
}

void snow_mutex_unlock(struct snow_mutex_t *mutex)
{
	if (wait_put(&mutex->list, (void *)mutex_put, mutex))
		snow_sched();
}

void snow_sem_init(struct snow_sem_t *sem, char *name, int count)
{
	sem->name = name;
	sem->count = count;
	list_init(&sem->list);
}

static int sem_get(struct snow_sem_t *sem)
{
	if (sem->count > 0) {
		sem->count--;
		return 1;
	}
	return 0;
}

static int sem_put(struct snow_sem_t *sem)
{
	sem->count++;
	return 1;
}

int snow_sem_down_timeout(struct snow_sem_t *sem, unsigned int us)
{
	return wait_get_timeout(&sem->list, us, (void *)sem_get, sem);
}

void snow_sem_up(struct snow_sem_t *sem)
{
	if (wait_put(&sem->list, (void *)sem_put, sem))
		snow_sched();
}

void snow_timer_init(struct snow_timer_t *timer, char *name, void(*timer_fn)(void *), void *arg)
{
	timer->name = name;
	timer->timer_fn = timer_fn;
	timer->arg = arg;
	list_init(&timer->list);
}

void snow_timer_add(struct snow_timer_t *timer, unsigned int us)
{
	struct list_t *l, *tmp, *s;
	struct snow_timer_t *t;

	timer->us = snow_gettime_us() + us;
	interrupt_disable();
	if (list_empty(&timer_list))
		list_fifo_put(&timer->list, &timer_list);
	else {
		s = &timer_list;
		list_foreach(l, tmp, &timer_list) {
			t = holder_of(l, struct snow_timer_t, list);
			if (timer->us > t->us)
				s = l;
			else
				break;
		}
		list_insert(&timer->list, s, s->next);
	}
	interrupt_enable();
}

void snow_timer_del(struct snow_timer_t *timer)
{
	interrupt_disable();
	list_remove(&timer->list);
	interrupt_enable();
}

void *snow_thread_create(char *name, void(*thread_fn)(void *),
		void *arg, void *sp, int stack_size)
{
	struct snow_thread_t *thread;

	if (stack_size < SNOW_SYS_STACK) {
		arch_print("Error: stack size too small\n");
		return NULL;
	}
	thread = sp;
	thread->stack = (void *)((unsigned long)sp + stack_size - sizeof(long));
	thread->sp = sp;
	thread->stack_size = stack_size;
	thread->name = name;
	thread->thread_fn = thread_fn;
	thread->thread_arg = arg;
	thread->sched_us = SNOW_THREAD_SCHED_US;
	snow_timer_init(&thread->wakeup_timer, name, (void(*)(void *))snow_wakeup, thread);
	arch_syscall(SNOW_SYSCALL_FORKIDLE, (long)thread);
	interrupt_disable();
	thread_set_flag(thread, FLAGS_RUN);
	list_fifo_put(&thread->list, &sched_list);
	interrupt_enable();
	return thread;
}

void snow_sched(void)
{
	arch_syscall(SNOW_SYSCALL_SCHED, 0);
}

void snow_start(void(*main)(void *), void *main_sp, int main_stack_size, void *idle_stack)
{
	interrupt_disable();
	arch_init();
	arch_print("MiniSnow V0.1 Powered by Yang, Bin <bin.yang@intel.com>\n");

	current = idle = (struct snow_thread_t *)((unsigned long)idle_stack - SNOW_SYS_STACK);
	memset(idle, 0, sizeof(struct snow_thread_t));
	idle->name = "idle";
	idle->stack = idle_stack;
	interrupt_enable();

	snow_thread_create("main", main, NULL, main_sp, main_stack_size);
	while(1) {
		arch_print("idle loop...\n");
	}
}

void __snow_tick_handler(void *regs)
{
	sys_time_us += 1000000/SNOW_HZ;
}

void __snow_syscall_handler(void *regs)
{
	switch (arch_get_arg(regs, 0)) {
		case SNOW_SYSCALL_DUMMY:
			break;
		case SNOW_SYSCALL_SCHED:
			current->timeout = 0;
			break;
		case SNOW_SYSCALL_FORKIDLE:
		{
			struct snow_thread_t *thread_new = (void *)arch_get_arg(regs, 1);
			char *sp_idle = idle->sp;
			char *stack_idle = idle->stack;
			char *stack_new = thread_new->stack;
			char *sp_new = stack_new - (stack_idle - sp_idle);

			memcpy(sp_new, sp_idle, stack_idle - sp_idle);
			arch_set_sp(sp_new, (long)sp_new);
			arch_set_arg(sp_new, 0, (long)thread_new);
			arch_set_pc(sp_new, (long)init_thread);
			thread_new->sp = sp_new;
			break;
		}
		default:
			arch_assert(0, "bad syscall\n");
			break;
	}
}

void __snow_sched(void)
{
	struct list_t *list;
	struct snow_timer_t *timer;
	unsigned long long systime;

	interrupt_disable();
	systime = snow_gettime_us();

	while (!list_empty(&timer_list)) {
		list = timer_list.next;
		timer = holder_of(list, struct snow_timer_t, list);
		if (timer->us <= systime) {
			list_remove(list);
			timer->timer_fn(timer->arg);
		} else
			break;
	}

	if (current->timeout < systime || !thread_is_run(current) || current == idle) {
		if (thread_is_run(current) && current != idle)
			list_fifo_put(&current->list, &sched_list);
		if (!list_empty(&sched_list)) {
			list = list_fifo_get(&sched_list);
			current = holder_of(list, struct snow_thread_t, list);
		} else
			current = idle;
		current->timeout = systime + current->sched_us;
	}

	interrupt_enable();
}

