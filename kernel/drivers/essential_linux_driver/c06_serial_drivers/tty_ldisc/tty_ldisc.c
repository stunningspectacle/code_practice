struct tty_ldisc_ops ldisc_ops = {
	TTY_LDISC_MAGIC,         /* Magic */
	"n_tch",                 /* Name of the line discipline */
	N_TCH,                   /* Line discipline ID number */
	n_touch_open,            /* Open the line discipline */
	n_touch_close,           /* Close the line discipline */
	n_touch_flush_buffer,    /* Flush the line discipline's read
				    buffer */
	n_touch_chars_in_buffer, /* Get the number of processed characters in
				    the line discipline's read buffer */
	n_touch_read,            /* Called when data is requested
				    from user space */
	n_touch_write,           /* Write method */
	n_touch_ioctl,           /* I/O Control commands */
	NULL,                    /* We don't have a set_termios
				    routine */
	n_touch_poll,            /* Poll */
	n_touch_receive_buf,     /* Called by the low-level driver
				    to pass data to user space*/
	n_touch_receive_room,    /* Returns the room left in the line
				    discipline's read buffer */
	n_touch_write_wakeup     /* Called when the low-level device
				    driver is ready to transmit more
				    */
};

struct tty_struct mytty_struct = {

};
struct tty_ldisc n_touch_ldisc = {
	.ops = &ldisc_ops,
	.tty = &mytty_struct,
};

if ((err = tty_register_ldisc(N_TCH, &n_touch_ldisc))) {
	return err;
}
