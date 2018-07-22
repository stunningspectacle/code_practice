
#ifndef _HELLO_H_
#define _HELLO_H_

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */


#ifndef HELLO_MAJOR
#define HELLO_MAJOR 0   /* dynamic major by default */
#endif

#ifndef HELLO_NR_DEVS
#define HELLO_NR_DEVS 4    /* hello0 through hello3 */
#endif


/*
 * The different configurable parameters
 */
extern int hello_major;     /* hello.c */
extern int hello_nr_devs;


/*
 * Prototypes for shared functions
 */

int hello_open(struct inode *inode, struct file *file);
int hello_release(struct inode *inode, struct file *file);
ssize_t hello_read(struct file *file, char *buf, size_t count, loff_t *ptr);
ssize_t hello_write(struct file *file, const char *buf, size_t count, loff_t * ppos);
int hello_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);

struct hello_dev {
	struct cdev cdev;	  /* Char device structure		*/
};


#endif /* _HELLO_H_ */
