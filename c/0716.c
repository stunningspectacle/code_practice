#define local_irq_disable() \
	do { raw_local_hardirq_disable(); trace_hardirq_disable(); } while (0)
