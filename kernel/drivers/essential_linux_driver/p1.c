DECLARE_WAIT_QUEUE_HEAD(myevent_wq);
rwlock_t myevent_lock;
extern unsigned int myevent_id;

static int mythread(void *unused)
{
    unsigned int event_id = 0;
    DECLARE_WAITQUEUE(wait, current);
    daemonize("mythread");

    allow_signal(SIGKILL);
    add_wait_queue(&myevent_wq, &wait);

    for (;;) {
        set_current_state(TASK_INTERRUPTIBLE);
        schedule();
        //wait_event_interruptible(&myevent_wq, myevent_id || signal_pending(current));

        if (signal_pending(current))
            break;
        read_lock(&myevent_lock);
        if (myevent_id) {
            event_id = myevent_id;
            read_unlock(&myevent_lock);
            run_umode_handler(event_id);
        } else
            read_unlock(&myevent_lock);
    }

    set_current_state(TASK_RUNNING);
    remove_wait_queue(&myevent_wq, &wait);

    return 0;
}

static void run_usermode_helper(int event_id)
{
    int i = 0;
    char *argv[2], *envp[4], *buffer = NULL;
    int value;

    argv[i++] = myevent_handler;
    if (!buffer = kmalloc(32, GFP_KERNEL))
        return;
    sprintf(buffer, "TROUBLED_DS=%d", event_id);
    if (!argv[0])
        return;
    argv[i] = 0;

    i = 0;
    envp[i++] = "HOME=/";
    envp[i++] = "PATH=/sbin:/usr/sbin:/bin:/usr/bin";
    envp[i++] = buffer;
    envp[i] = 0;

    value = call_usermodehelper(argv[0], argv, envp, 0);

    kfree(buffer);
}

