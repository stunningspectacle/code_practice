/* Timekeeping */
#include <linux/param.h>
#include <linux/jiffies.h>

volatile unsigned long jiffies
u64 jiffies_64
/*
The jiffies_64 variable is incremented once for each clock tick; thus, it’s incre-
mented HZ times per second. Kernel code most often refers to jiffies, which is
the same as jiffies_64 on 64-bit platforms and the least significant half of it on
32-bit platforms.
*/
int time_after(unsigned long a, unsigned long b);
int time_before(unsigned long a, unsigned long b);
int time_after_eq(unsigned long a, unsigned long b);
int time_before_eq(unsigned long a, unsigned long b);
/*These Boolean expressions compare jiffies in a safe way, without problems in 
case of counter overflow and without the need to access jiffies_64.
*/
u64 get_jiffies_64(void);
/* Retrieves jiffies_64 without race conditions. */
#include <linux/time.h>
unsigned long timespec_to_jiffies(struct timespec *value);
void jiffies_to_timespec(unsigned long jiffies, struct timespec *value);
unsigned long timeval_to_jiffies(struct timeval *value);
void jiffies_to_timeval(unsigned long jiffies, struct timeval *value);
/* Converts time representations between jiffies and other representations. */
#include <asm/msr.h>
rdtsc(low32,high32);
rdtscl(low32);
rdtscll(var32);
/*
x86-specific macros to read the timestamp counter. They read it as two 32-bit
halves, read only the lower half, or read all of it into a long long variable.
*/
#include <linux/timex.h>
cycles_t get_cycles(void);
/*
Returns the timestamp counter in a platform-independent way. If the CPU offers
no timestamp feature, 0 is returned.
*/
#include <linux/time.h>
unsigned long mktime(year, mon, day, h, m, s);
Returns the number of seconds since the Epoch, based on the six unsigned int arguments.
void do_gettimeofday(struct timeval *tv);
/*
Returns the current time, as seconds and microseconds since the Epoch, with the
best resolution the hardware can offer. On most platforms the resolution is one
microsecond or better, although some platforms offer only jiffies resolution.
*/
struct timespec current_kernel_time(void);
/* Returns the current time with the resolution of one jiffy. */

/* Delays */
#include <linux/wait.h>
long wait_event_interruptible_timeout(wait_queue_head_t *q, condition, signed long timeout);
/*
Puts the current process to sleep on the wait queue, installing a timeout value
expressed in jiffies. Use schedule_timeout (below) for noninterruptible sleeps.
*/
#include <linux/sched.h>
signed long schedule_timeout(signed long timeout);
/*
Calls the scheduler after ensuring that the current process is awakened at time-
out expiration. The caller must invoke set_current_state first to put itself in an
interruptible or noninterruptible sleep state.
*/

#include <linux/delay.h>
void ndelay(unsigned long nsecs);
void udelay(unsigned long usecs);
void mdelay(unsigned long msecs);
/*
Introduces delays of an integer number of nanoseconds, microseconds, and mil-
liseconds. The delay achieved is at least the requested value, but it can be more.
The argument to each function must not exceed a platform-specific limit (usually a few thousands).
*/
void msleep(unsigned int millisecs);
unsigned long msleep_interruptible(unsigned int millisecs);
void ssleep(unsigned int seconds);
/*
Puts the process to sleep for the given number of milliseconds (or seconds, in the case of ssleep).
*/

Kernel Timers
#include <asm/hardirq.h>
int in_interrupt(void);
int in_atomic(void);
/*
Returns a Boolean value telling whether the calling code is executing in inter-
rupt context or atomic context. Interrupt context is outside of a process con-
text, either during hardware or software interrupt processing. Atomic context is
when you can’t schedule either an interrupt context or a process’s context with a
spinlock held.
*/
#include <linux/timer.h>
void init_timer(struct timer_list * timer);
struct timer_list TIMER_INITIALIZER(_function, _expires, _data);
/*
This function and the static declaration of the timer structure are the two ways
to initialize a timer_list data structure.
*/
void add_timer(struct timer_list * timer);
/* Registers the timer structure to run on the current CPU. */
int mod_timer(struct timer_list *timer, unsigned long expires);
/* Changes the expiration time of an already scheduled timer structure. It can also act as an alternative to add_timer. */
int timer_pending(struct timer_list * timer);
/* Macro that returns a Boolean value stating whether the timer structure is already registered to run. */
void del_timer(struct timer_list * timer);
void del_timer_sync(struct timer_list * timer);
/*
Removes a timer from the list of active timers. The latter function ensures that
the timer is not currently running on another CPU.
*/

/* Tasklets */
#include <linux/interrupt.h>
DECLARE_TASKLET(name, func, data);
DECLARE_TASKLET_DISABLED(name, func, data);
void tasklet_init(struct tasklet_struct *t, void (*func)(unsigned long),
		  unsigned long data);
/*
The first two macros declare a tasklet structure, while the tasklet_init function
initializes  a  tasklet  structure  that  has  been  obtained  by  allocation  or  other
means. The second DECLARE macro marks the tasklet as disabled.
*/
void tasklet_disable(struct tasklet_struct *t);
void tasklet_disable_nosync(struct tasklet_struct *t);
void tasklet_enable(struct tasklet_struct *t);
/*
Disables and reenables a tasklet. Each disable must be matched with an enable
(you can disable the tasklet even if it’s already disabled). The function tasklet_
disable waits for the tasklet to terminate if it is running on another CPU. The
nosync version doesn’t take this extra step.
*/
void tasklet_schedule(struct tasklet_struct *t);
void tasklet_hi_schedule(struct tasklet_struct *t);
/*
Schedules a tasklet to run, either as a “normal” tasklet or a high-priority one.
When  soft  interrupts  are  executed,  high-priority  tasklets  are  dealt  with  first,
while normal tasklets run last.
*/
void tasklet_kill(struct tasklet_struct *t);
/*
Removes the tasklet from the list of active ones, if it’s scheduled to run. Like
tasklet_disable, the function may block on SMP systems waiting for the tasklet to
terminate if it’s currently running on another CPU.
*/

/* Workqueues */
#include <linux/workqueue.h>
struct workqueue_struct;
struct work_struct;
/* The structures representing a workqueue and a work entry, respectively. */
struct workqueue_struct *create_workqueue(const char *name);
struct workqueue_struct *create_singlethread_workqueue(const char *name);
void destroy_workqueue(struct workqueue_struct *queue);
/*
Functions  for  creating  and  destroying  workqueues.  A  call  to  create_work-
queue creates a queue with a worker thread on each processor in the system;
instead,  create_singlethread_workqueue  creates  a  workqueue  with  a  single
worker process.
*/
DECLARE_WORK(name, void (*function)(void *), void *data);
INIT_WORK(struct work_struct *work, void (*function)(void *), void *data);
PREPARE_WORK(struct work_struct *work, void (*function)(void *), void *data);
/* Macros that declare and initialize workqueue entries. */
int queue_work(struct workqueue_struct *queue, struct work_struct *work);
int queue_delayed_work(struct workqueue_struct *queue, struct work_struct
		  *work, unsigned long delay);
/* Functions that queue work for execution from a workqueue. */
int cancel_delayed_work(struct work_struct *work);
void flush_workqueue(struct workqueue_struct *queue);
/*
Use cancel_delayed_work to remove an entry from a workqueue; flush_workqueue
ensures that no workqueue entries are running anywhere in the system.
*/
int schedule_work(struct work_struct *work);
int schedule_delayed_work(struct work_struct *work, unsigned long delay);
void flush_scheduled_work(void);
/* Functions for working with the shared workqueue. */
