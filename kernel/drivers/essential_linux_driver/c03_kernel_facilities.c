#if 0
Table 3.3. Summary of Data Structures
wait_queue_entry    include/linux/wait.h Used by threads that desire to wait for an event or a system resource
list_head           include/linux/list.h Kernel structure to weave a doubly linked list of data structures
hlist_head          include/linux/list.h Kernel structure to implement hash tables
work_struct         include/linux/workqueue.h Implements work queues, which are a way to defer work inside the kernel
notifier_block      include/linux/notifier.h Implements notifier chains, which are used to send status changes to code regions that request them
completion          include/linux/completion.h Used to initiate activities as separate threads and then wait for them to complete

Table 3.4. Summary of Kernel Programming InterfacesKernel Interface Location Description
DECLARE_WAITQUEUE()             include/linux/wait.h Declares a wait queue.
add_wait_queue()                kernel/wait.c Queues a task to a wait queue.  The task goes to sleep until it's woken up by another thread or interrupt handler.
remove_wait_queue()             kernel/wait.c Dequeues a task from a wait queue.
wake_up_interruptible()         include/linux/wait.h kernel/sched.c Wakes up a task sleeping inside a wait queue and puts it back into the scheduler run queue.
schedule()                      kernel/sched.c Relinquishes the processor and allows other parts of the kernel to run.
set_current_state()             include/linux/sched.h Sets the run state of a process.
                                The state can be one of TASK_RUNNING, TASK_INTERRUPTIBLE, TASK_UNINTERRUPTIBLE, TASK_STOPPED, TASK_TRACED, EXIT_ZOMBIE, or EXIT_DEAD.
kernel_thread()                 arch/your- arch/kernel/process.c Creates a kernel thread.
daemonize()                     kernel/exit.c Activates a kernel thread without attaching user resources and changes the parent of the calling thread to kthreadd.
allow_signal()                  kernel/exit.c Enables delivery of a specified signal.
signal_pending()                include/linux/sched.h Checks whether a signal has been delivered.
                                There are no signal handlers inside the kernel, so you have to explicitly check whether a signal has been delivered.
call_usermodehelper()           include/linux/kmod.h kernel/kmod.c Executes a user mode program.
Linked list library functions   include/linux/list.h See Table 3.1.
register_die_notifier()         arch/your-arch/kernel/traps.c Registers a die notifier.
register_netdevice_notifier()   net/core/dev.c Registers a netdevice notifier.
register_inetaddr_notifier()    net/ipv4/devinet.c Registers an inetaddr notifier.
BLOCKING_NOTIFIER_HEAD()        include/linux/notifier.h       Creates a user-defined blocking notifier.
blocking_notifier_chain_register() kernel/sys.c Registers a blocking notifier.Kernel Interface Location Description
blocking_notifier_call_chain()     kernel/sys.c Dispatches an event to a blocking notifier chain.
ATOMIC_NOTIFIER_HEAD()          include/linux/notifier.h       Creates an atomic notifier.
atomic_notifier_chain_register()   kernel/sys.c Registers an atomic notifier.
DECLARE_COMPLETION()            include/linux/completion.h Statically declares a completion object.
init_completion()               include/linux/completion.h Dynamically declares a completion object.
complete()                      kernel/sched.c Announces completion.
wait_for_completion()           kernel/sched.c Waits until the completion object completes.
complete_and_exit()             kernel/exit.c Atomically signals completion and exit.
kthread_create()                kernel/kthread.c Creates a kernel thread.
kthread_stop()                  kernel/kthread.c Asks a kernel thread to stop.
kthread_should_stop()           kernel/kthread.c A kernel thread can poll on this function to detect whether another thread has asked it to stop via kthread_stop().
IS_ERR()                        include/linux/err.h Finds out whether the return value is an error code.
blocking_notifier_call_chain()  kernel/sys.c Dispatches an event to a blocking notifier chain.
ATOMIC_NOTIFIER_HEAD()          include/linux/notifier.h       Creates an atomic notifier.
atomic_notifier_chain_register()   kernel/sys.c Registers an atomic notifier.
DECLARE_COMPLETION()            include/linux/completion.h Statically declares a completion object.
init_completion()               include/linux/completion.h Dynamically declares a completion object.
complete()                      kernel/sched.c Announces completion.
wait_for_completion()           kernel/sched.c Waits until the completion object completes.
complete_and_exit()             kernel/exit.c Atomically signals completion and exit.
kthread_create()                kernel/kthread.c Creates a kernel thread.
kthread_stop()                  kernel/kthread.c Asks a kernel thread to stop.
kthread_should_stop()           kernel/kthread.c A kernel thread can poll on this function to detect whether another thread has asked it to stop via kthread_stop().
IS_ERR()                        include/linux/err.h Finds out whether the return value is an error code.
#endif

#include <linux/list.h>

static struct _mydrv_wq {
    struct list_head mydrv_worklist;
    spinlock_t lock;
    wait_queue_head_t todo;
} mydrv_wq;

static struct _mydrv_work {
    struct list_head mydrv_workitem;
    void (*worker_func)(void *);
    void *worker_data;
} mydrv_work;

/* Listing 3.2 */
static int __init mydrv_init(void)
{
    spin_lock_init(&mydrv_wq.lock);

    init_waitqueue_head(&mydrv_wq.todo);

    INIT_LIST_HEAD(&mydrv_wq.mydrv_worklist);

    kernel_thread(mydrv_worker, NULL,
            CLONE_FS|CLONE_FILES|CLONE_SIGHAND|SIGCHLD);

    reutrn 0;
}

/* Listing 3.3 */
int submit_work(void (*func)(void *data), void *data);
{
    struct _mydrv_work *mydrv_work;
    mydrv_work = kmalloc(sizeof(*mydrv_work), GFP_ATOMIC);
    if (!mydrv_work)
        return -1;
    mydrv_work->worker_func = func;
    mydrv_work->worker_data = data;

    spin_lock(&mydrv_wq.lock);
    list_add_tail(&mydrv_work->mydrv_workitem, &mydrv_wq.mydrv_worklist);
    wake_up(&mydrv_wq.todo);
    spin_unlock(&mydrv_wq.lock);

    return 0;
}

/* Listing 3.4 */
static int mydrv_worker(void *unused)
{
    DECLARE_WAITQUEUE(wait, current);
    void (*worker_func)(void *);
    void *worker_data;
    struct _mydrv_work *mydrv_work;

    set_current_state(TASK_INTERRUPTIBLE);

    while (!asked_to_die()) {
        add_wait_queue(&mydrv_wq.todo, &wait);
        if (list_empty(&mydrv_wq.mydrv_worklist))
            schedule();
        else
            set_current_state(TASK_RUNNING);
        remove_wait_queue(&mydrv_wq.todo, &wait);

        spin_lock(&mydrv_wq.lock);
#ifdef NOT_USE_LIST_FOR_EACH_ENTRY_SAFE
        while (!list_empty(&mydrv_wq.mydrv_worklist)) {
            mydrv_work = list_entry(mydrv_wq.mydrv_worklist.next,
                    struct _mydrv_work, mydrv_workitem);
#else
        struct _mydrv_work *tmp;
        list_for_each_entry_safe(mydrv_work,
                tmp, &mydrv_wq.mydrv_worklist, mydrv_workitem) {
#endif
            worker_func = mydrv_work->worker_func;
            woker_data = mydrv_work->worker_data;

            list_del(mydrv_wq.mydrv_worklist.next);
            spin_unlock(&mydrv_wq.lock);

            kfree(mydrv_work);
            worker_func(worker_data);

            spin_lock(&mydrv_wq.lock);
        }

        spin_unlock(&mydrv_wq.lock);
        set_current_state(TASK_INTERRUPTIBLE);
    }

    set_current_state(TASK_RUNNING);
    return 0;
}


#define container_of(ptr, type, member) \
    (type *)((void *)ptr - (void *)&((type *)0)->member)
#define list_entry(ptr, type, member) container_of(ptr, type, member)

#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)
#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*pos), member)
#define list_last_entry(pos, member) \
    list_entry((pos)->member.prev, typeof(*pos), member)

#define list_for_each_entry(pos, head, member) \
    for (pos = list_first_entry(head, typeof(*pos), member); \
            pos->member != head; \
            pos = list_next_entry(pos, member))
#define list_for_each_entry_safe(pos, n, head, member) \
    for (pos = list_first_entry(head, typeof(*pos), member), \
            n = list_next_entry(pos, member); \
            &(pos)->member != (head); \
            pos = n, n = list_next_entry(n, member))
            
/* Listing 3.5 */
#include <linux/workqueue.h>
struct workqueue_struct *wq;

static int __init mydrv_init_wq(void)
{
    wq = create_singlethread_workqueue("mydrv");

    return 0;
}
int submit_work_wq(void (*func)(void *), void *data)
{
    struct work_struct *hardwork;

    hardwork = kmalloc(sizeof(*hardwork), GFP_KERNEL);

    INIT_WORK(hardwork, func);
    atomic_set(&hardwork->data, data);

    queue_work(wq, hardwork);

    return 0;
}

/* Listing 3.6 Notifier Event Handlers */
#include <linux/notifier.h>
#include <asm/kdebug.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>

static struct notifier_block my_die_notifier = {
    .notifier_call = my_die_event_handler,
};
static int my_die_event_handler(struct notifier_block *self,
        unsigned long val, void *data)
{
    struct die_args *args = (struct die_args *)data;

    if (val == DIE_OOPS)
        printk("my_die_event: OOPs @ EIP=%lx\n", args->regs->eip);
    return 0;
}

static struct notifier_block my_dev_notifier = {
    .notifier_call = my_dev_event_handler,
};
static int my_dev_event_handler(struct notifier_block *self,
        unsigned long val, void *data)
{
    printk("my_dev_event: val=%ld, interface=%s\n",
            val, ((struct net_device *)data)->name);
    return 0;
}

static BLOCKING_NOTIFIER_HEAD(my_noti_chain);
static struct notifier_block my_notifier = {
    .notifier_call = my_event_handler,
};
static int my_event_handler(struct notifier_block *self,
        unsigned long val, void *data)
{
    printk("my_event: val=%ld\n", val);
    return 0;
}

static int __init myevent_init(void)
{
    register_die_notifier(&my_die_notifier);

    register_netdevice_notifier(&my_dev_notifier);

    blocking_notifier_chain_register(&mynoti_chain, &my_notifier);
}

/* Listing 3.7. Synchronizing Using Completion */
DECLARE_COMPLETION(my_thread_exit);
DECLARE_WAIT_QUEUE_HEAD(my_thread_wait);
int pink_slip = 0;

static int my_thread(void *unused)
{
    DECLARE_WAITQUEUE(wait, current);

    add_wait_queue(&my_thread_wait, &wait);
    while (1) {
        set_current_state(TASK_INTERRUPTIBLE);
        if (pink_slip)
            break;

        schedule();

    }

    __set_current_state(TASK_RUNNING);
    remove_wait_queue(&my_thread_wait, &wait);
#ifdef COMPLETE_AND_EXIT
    complete_and_exit(&my_thread_exit, 0);
#else
    complete(&my_thread_exit);
    do_exit(0);
#endif
}

static int __init my_init(void)
{
#ifdef KTHREAD_CREATE
    kernel_thread(my_thread, NULL, CLONE_FS | CLONE_FILES | CLONE_SIGHAND | SIGCHLD);
#else
    kthread_run(my_thread, NULL, "my_thread");
#endif

    return 0;
}

static void __exit my_release(void)
{
    pink_slip = 1;
    wake_up(&my_thread_wait); /* wake_up_interruptible() ? */
    wait_for_completion(&my_thread_exit);
}

/* Listing 3.8. Synchronizing Using Kthread Helpers */
#include <linux/kthread.h>

struct task_struct *my_task;
static int my_thread(void *unused)
{
    while (1) {
        set_current_state(TASK_INTERRUPTIBLE); // ?
        if (kthread_should_stop())
            break;
        /* do something */

        schedule();
    }
    __set_current_state(TASK_RUNNING); // ?
}

static int __init my_init(void)
{
#ifdef KTHREAD_RUN
    kthread_run(my_thread, NULL, "my_thread");
#else

    my_task = kthread_create(my_thread, NULL, "my_thead");
    if (my_task)
        wake_up_process(my_task);
#endif

    return 0;
}

static void __exit my_release(void)
{
    kthread_stop(my_task);
}

/* Listing 3.9. Using Error_Handling Aids */
#include <linux/err.h>

char * collect_data(char *userbuffer)
{
    char *buffer;

    buffer = kmalloc(100, GFP_KERNEL);
    if (!buffer) 
        return ERR_PTR(-ENOMEM);
    if (copy_from_user(buffer, userbuffer, 100))
        return ERR_PTR(-EFAULT);

    return buffer;
}

int my_function(char *userbuffer)
{
    char *buf;
    struct task_struct *my_task;

    my_task = kthread_create(my_thread, NULL, "my_thread");

    buf = collect_data(userbuffer);
    if (!IS_ERR(buf))
        wake_up_process(my_task);
    else
        printk("Error returned is %d!\n", PTR_ERR(buf));

    return 0;
}

