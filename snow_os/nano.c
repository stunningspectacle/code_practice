#define FLAGS_RUN_MASK	0xff
#define FLAGS_RUN	(1 << 0)
#define FLAGS_WAIT	(1 << 1)

#define STACK_SIZE	1024
#define HZ 100

typedef struct list {
	struct list *prev, *next;
} list_t;

#define container_of(ptr, type, member) \
	(type *)((char *)(ptr) - (char *)&(((type *)0)->member))


#define list_foreach(cur, next, list_head) \
	for (cur = (list_head)->next, next = cur->next; \
			cur != (list_head); \
			cur = next, next = cur->next)

#define LIST_HEAD(name) list_t name = {&name, &name};

typedef struct mutex {
	char *name;
	void *thread;
	int count;
	list_t list;
} mutex_t;

typedef struct sem {
	char *name;
	int count;
	list_t list;
} sem_t;

typedef timer {
	char *name;
	uint64_t us;
	void (*timer_fn)(void *);
	void *arg;
	list_t list;
} timer_t;

typedef struct thread {
	char *sp;
	char *name;
	uint32_t flags;
	void (*thread_fn)(void *);
	void *thread_arg;
	int stack_size;
	char *stack;
	list_t list;
	uint64_t timeout;
	uint32_t sched_us;
	timer_t wakeup_timer;
} thread_t;

typedef void (*thread_fn)(void *) thread_func_t;

LIST_HEAD(timer_list);
LIST_HEAD(sched_list);

static inline void list_init(list_t *list)
{
	list->prev = list;
	list->next = list;
}

static inline void list_insert(list_t *new, list_t *prev, list_t *next)
{
	prev->next = new;
	new->prev = prev;
	new->next = next;
	next->prev = new;
}

static inline void list_remove(list_t *list)
{
	list_t *prev = list->prev;
	list_t *next = list->next;

	prev->next = next;
	next->prev = prev;
	list_init(list);
}

static inline int list_empty(list_t *list)
{
	return list->next == list;
}

static inline void list_fifo_put(list_t *new, list_t *fifo)
{
	list_insert(new, fifo->prev, fifo)
}

static inline list_t *list_fifo_get(list_t *fifo)
{
	list_t *list;

	if (list_empty(fifo))
		return NULL;
	list = fifo->next;
	list_remove(list);

	return list;
}

static inline thread_is_run(thread_t *thread)
{
	return thread->flags & FLAGS_RUN;
}

static inline thread_is_wait(thread_t *thread)
{
	return thread->flags & FLAGS_WAIT;
}

static void thread_set_flag(thread_t *thread, int flag)
{
	if (flag & FLAGS_RUN_MASK)
		thread->flags &= ~FLAGS_RUN_MASK;
	thread->flags |= flag;
}

static void interrupt_enable(void)
{
	interrupt_disable_depth--;
	if (interrupt_disable_depth < 0)
		printk("unbalance interrupt depth\n");
	if (interrupt_disable_depth == 0)
		arch_enable_interrupt();
}

static void interrupt_disable(void)
{
	if (interrupt_disable_depth == 0)
		arch_disable_interrupt();
	interrupt_disable_depth++;
}

uint64_t k_gettime_us(void)
{
	uint64_t res;

	interrupt_disable();
	res = sys_time_us;
	interrupt_enable();

	return res;
}

void k_udelay(uint32_t us)
{
	uint64_t start = k_gettime_us();

	while (k_gettime_us() - start < us)
		;
}

void k_timer_del(timer_t *timer)
{
	interrupt_disable();

	list_remove(&timer->list);

	interrupt_enable();
}

void k_timer_add(timer_t *timer, uint32_t us)
{
	list_t *cur, *tmp, *s;
	timer_t *t;

	timer->us = k_gettime_us() + us;
	interrupt_disable();
	if (list_empty(&timer_list))
		list_fifo_put(&timer->list, &timer_list);
	else {
		// insert the list between s and s->next
		s = &timer_list->prev;
		list_foreach(cur, tmp, &timer_list) {
			t = container_of(cur, timer_t, list);
			if (timer->us < t->us ) {
				s = cur->prev;
				break;
			}
		}
		list_insert(&timer->list, s, s->next);
	}
	interrupt_enable();
}

void init_thread(thread_t *thread)
{
	printk("init new thread\n");
	thread->thread_fn(thread->thread_arg);
	printk("thread end ...\n");

	while (1);
}

void k_wakeup(thread_t *thread)
{
	interrupt_disable();
	if (!thread_is_run(thread)) {
		thread_set_flag(thread, FLAGS_RUN);
		list_remove(&thread->list);
		list_fifo_put(&thread->list, &sched_list);
	}
	k_timer_del(&thread->wakeup_timer);
	interrupt_enable();
}

void k_usleep(uint32_t us)
{
	if (current == idle) {
		printk("Error: cannot sleep in idle thread\n");
		return;
	}

	interrupt_disable();
	thread_set_flag(current, FLAGS_SLEEPING);
	k_timer_add(&current->wakeup_timer, us);
	interrupt_enable();

	sched();
}

static int wait_get_timeout(list_t *wait_list, uint32_t us, int (*func)(void *), void *args)
{
}

void sem_init(sem_t *sem, char *name, int count)
{
	sem->name = name;
	sem->count = 0;
	list_init(&sem->list);
}

static int sem_down(sem_t *sem)
{
	if (sem->count > 0) {
		sem->count--;
		return 1;
	}
	return 0;
}

int sem_down_timeout(sem_t *sem, uint32_t us)
{
	int ret = 0;
	uint64_t timeout, cur_time;
	int need_wakeup = 0;

	if (us == 0) {
		timeout = 0;
		need_wakeup = 0;
	} else if (use == -1) {
		timeout = -1;
		need_wakeup = 1;
	} else {
		timeout = k_gettime_us() + us;
		need_wakeup = 1;
	}

again:
	interrupt_disable();
	ret = sem_down(sem);
	if (ret)
		return ret;

	cur_time = k_gettime_us();
	if (cur_time > timeout)
		goto fail;

	if (need_wakeup)
		k_timer_add(&current->wakeup_timer, timeout - cur_time);
	list_fifo_put(&current->thread_list, &sem->list);
	thread_set_flag(current, FLAGS_WAIT);
	interrupt_enable();
	sched();
	goto again;

fail:
	interrupt_enable();
	return 0;
success:
	interrupt_enable();
	return 1;
}


static void sem_up(sem_t *sem)
{
	list_t *l;
	thread_t *thread;
	int need_sched = 0;

	interrupt_disable();
	sem->count++; 

	if (!list_empty(&sem->list)) {
		l = list_fifo_get(&sem->list);
		thread = container_of(l, thread_t, thread_list);
		k_wakeup(thread);
		need_sched = 1;
	}
	interrupt_enable();

	if (need_sched)
		sched();
}

void syscall_handler(void *regs)
{
	switch (arch_get_arg(regs, 0)) {
	case SYSCALL_DUMMY:
		break;
	case SYSCALL_SCHED:
		current->timeout = 0;
		break;
	case SYSCALL_FORKIDLE:
		thread_t *thread = (void *)arch_get_arg(regs, 1);
		char *sp_idle = idle->sp;
		char *stack_idle = idle->stack;
		char *stack_new = thread->stack;
		char *sp_new = stack_new - (stack_idle - sp_idle);

		memcpy(sp_new, sp_idle, stack_idle - sp_idle);
		arch_set_sp(sp_new, (uint32_t)sp_new);
		arch_set_arg(sp_new, 0, (uint32_t)thread);
		arch_set_pc(sp_new, (uint32_t)init_thread);
		thread->sp = sp_new;
		break;
	}
	default:
		printk("bad syscall\n");
		break;
}
	

void *k_thread_create(char *name,
		thread_func_t func, void *args, void *sp, int stack_size)
{
	thread_t *thread;

	if (stack_size < STACK_SIZE) {
		printk("Error, stack size too small\n");
		return NULL;
	}

	thread = (thread_t *)sp;
	thread->stack = (void *)((uint32_t)sp + stack_size - sizeof(uint32_t));
	thread->sp = sp;
	thread->stack_size = stack_size;
	thread->name = name;
	thread->thread_fn = func;
	thread->thread_arg = arg;
	thread->sched_us = THREAD_SCHED_US;
	k_timer_init(&thread->wakeup_timer, name, k_wakeup, thread);

	arch_syscall(SYSCALL_FORKIDLE, (long)thread);
	interrupt_disable();
	thread_set_flag(thread, FLAGS_RUN);
	list_fifo_put(&thread->list, &sched_list);
	interrupt_enable();

	return thread;
}

void k_start(thread_func_t main, void *main_sp, int main_stack_size, void *idle_task)
{
	interrupt_disable();
	arch_init();
	printk("Nano V0.1\n");

	current = idle = (thread_t *)((uint32_t)idle_stack - STACK_SIZE);
	memset(idle, 0, sizeof(thread_t));
	idle->name = "idle";
	idle->stack = idle_stack;
	interrupt_enable();

	thread_create("main", main, NULL, main_sp, main_stack_size);
	while(1) {
		printk("idle loop... \n");
	}
}

void tick_handler(void *regs)
{
	sys_time_us += 1000000 / HZ;
}

void sched(void)
{
	list_t *list;
	timer_t *timer;
	uint64_t systime;

	interrupt_disable();
	systime = k_gettime_us();

	while (!list_empty(&timer_list)) {
		list = timer_list.next;
		timer = container_of(list, list_t, list);
		if (timer->us > systime)
			break;
		list_remove(list);
		timer->timer_fn(timer->arg);
	}

	if (current->timeout < systime ||
		!thread_is_run(current) || current == idle) {
		if (thread_is_run(current) && current != idle)
			list_fifo_put(&current->list, &sched_list);
		if (!list_empty(&sched_list)) {
			list = list_fifo_get(&sched_list);
			current = container_of(list, thread_t, list);
		} else
			current = idle;
		current->timeout = systime + current->sched_us;
	}
	interrupt_enable();
}

__asm__ __volatile__(
		".type entry, %function"
		"entry:"
		"sub sp, #40"
		"mov r3, sp"
		"stmia sp, {r3 - r11, lr}"
		"mov r0, sp"
		"ldr r1, ="#handler
		"bl entry_handler"
		"ldmia r0, {r3 - r11, lr}"
		"mov sp, r3"
		"add sp, #40"
		"bx lr"
		)

	
