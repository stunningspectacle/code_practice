/* Listing 4.1. The Roller Interrupt Handler */
static DEFINE_SPINLOCK(roller_lock);
static DECLARE_WAIT_QUEUE_HEAD(roller_pull);

static irqreturn_t roller_interrupt(int irq, void *dev_id)
{
    int i, PA_t, PA_delta_t, movement = 0;

    PA_t = PORTD & 0x7;
    while (PA_t == PA_delta_t)
        PA_delta_t = PA_t & 0x7;

    movement = determine_movement(PA_t, PA_delta_t);

    spin_lock(&roller_lock);
    store_movements(movement);
    spin_unlock(&roller_lock);

    wake_up_interruptible(&roller_pull);

    return IRQ_HANDLED;
}

int determine_movement(int PA_t, int PA_delta_t)
{
    switch (PA_t) {
    case 0:
        switch (PA_delta_t) {
        case 1:
            movement = ANTICLOCKWISE;
            break;
        case 2:
            movement = CLOCKWISE;
            break;
        case 4:
            movement = KEYPRESSED;
            break;
        }
        break;
}

/* Listing 4.2. Using Softirqs to Offload Work from Interrupt Handlers */
static void roller_analyze(struct softirq_action *h)
{
}
void __init roller_init()
{
    /* Need to add ROLLER_SOFTIRQ into the enum to increase NR_SOFTIRQS */
    open_softirq(ROLLER_SOFTIRQ, roller_analyze);
}
static irqreturn_t roller_isr(int irq, void *dev_id)
{
    roller_capture();

    raise_softirq(ROLLER_SOFTIRQ);

    return IRQ_HANDLED;
}

/* Listing 4.3. Using Tasklet to Offload Work From Interrupt Handler */
struct roller_device_struct {
    struct tasklet_struct tsklt;
};
struct roller_device_struct roller_device;

static void roller_tasklet_func(void *)
{
}
static void __init roller_init()
{
    struct roller_device_struct *dev;

    dev = &roller_device;
    tasklet_init(&dev->tsklt, roller_tasklet_func, dev); 
}
static irqreturn_t roller_isr(int irq, void *dev_id)
{
    struct roller_device_struct *dev;

    roller_capture();

    tasklet_schedule(&dev->tsklt);

    return IRQ_HANDLED;
}


