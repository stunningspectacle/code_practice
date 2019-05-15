int do_exclusive()
{
	spin_lock(&lock);
	while (!something_is_ok()) {
		spin_unlock(&lock);
		if (!wait_event_interruptible(wq_head, something_is_ok()))
			return -ERESTARTSYS;
		spin_lock(&lock);
	}

	do_something();
	spin_unlock(&lock);
}

