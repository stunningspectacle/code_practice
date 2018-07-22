 /* Example Minimal Character Device Driver */ 
 #include <linux/module.h>
 #include <linux/init.h>
 #include <linux/moduleparam.h>

 #include <linux/kernel.h>	/* printk() */
 #include <linux/fs.h>          /* everything... */
 #include <linux/slab.h>	/* kmalloc() */
 #include <linux/types.h>	/* size_t */
 #include <linux/errno.h>	/* error codes */
 #include <linux/cdev.h>


 #include "hello.h"		/* local definitions */


 int hello_major =   HELLO_MAJOR;
 int hello_minor =   0;	
 int hello_nr_devs = HELLO_NR_DEVS;

 static int debug_enable = 0; /* Added driver parameter */ 
 module_param(debug_enable, int, 0); /* and these 2 lines */ 
 MODULE_PARM_DESC(debug_enable, "Enable module debug mode.");


 struct file_operations hello_fops; 
 struct hello_dev *hello_devices;	/* allocated in scull_init_module */

 
 struct file_operations hello_fops = { 
	.owner =   THIS_MODULE, 
	.read  =   hello_read, 
	.write =   hello_write, 
	.unlocked_ioctl =   hello_ioctl, 
	.open  =   hello_open, 
	.release = hello_release, 
  };
  

 int hello_open(struct inode *inode, struct file *file) 
 {
    	printk("hello_open: successful\n");

    	return 0;	

  }

 int hello_release(struct inode *inode, struct file *file) 
 { 
  	 printk("hello_release: successful\n"); 
   
  	 return 0; 
  } 

 ssize_t hello_read(struct file *file, char *buf, size_t count, loff_t *ptr) 
 { 
  	 printk("hello_read: returning zero bytes\n"); 
  
   	return 0; 
 } 
 
 ssize_t hello_write(struct file *file, const char *buf, size_t count, loff_t * ppos) 
 { 
   	printk("hello_write: accepting zero bytes\n"); 
 
   	return 0; 

  } 

  int hello_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) 
  { 

   	printk("hello_ioctl: cmd=%d, arg=%ld\n", cmd, arg); 
   
   	return 0; 

  }

  /*
   * Set up the char_dev structure for this device.
   */
  static void hello_setup_cdev(struct hello_dev *dev, int index)
  {
	int err, devno = MKDEV(hello_major, hello_minor + index);
    
	cdev_init(&dev->cdev, &hello_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &hello_fops;
	err = cdev_add(&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding hello%d", err, index);
  }

  void hello_exit(void) 
  { 
 	int i;
	dev_t devno = MKDEV(hello_major, hello_minor);
	
	/* Get rid of our char dev entries */
	if (hello_devices) {
		for (i = 0; i < hello_nr_devs; i++) {			
			cdev_del(&hello_devices[i].cdev);
		}
		kfree(hello_devices);
	}

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, hello_nr_devs);


    	printk("Hello Example Exit\n"); 

  } 
  
  int  hello_init(void) 
  { 

	int ret,i; 
	dev_t dev = 0;

	/*
	 * Get a range of minor numbers to work with, asking for a dynamic
	 * major unless directed otherwise at load time.
	 */
	printk("Hello Example Init - debug mode is %s\n", debug_enable ? "enabled" : "disabled"); 

	if (hello_major) {
		dev = MKDEV(hello_major, hello_minor);
		ret = register_chrdev_region(dev, hello_nr_devs, "hello");
	}else {
		ret = alloc_chrdev_region(&dev, hello_minor, hello_nr_devs,"hello");
		hello_major = MAJOR(dev);
	} 
	   
	if (ret < 0){ 		
		printk(KERN_WARNING "hello: can't get major %d\n", hello_major);
 		goto hello_fail; 
	} 

	/* 
	 * allocate the devices -- we can't have them static, as the number
	 * can be specified at load time
	 */
	hello_devices = kmalloc(hello_nr_devs * sizeof(struct hello_dev), GFP_KERNEL);
	if (!hello_devices) {
		ret = -ENOMEM;
		goto hello_fail;  /* Make this more graceful */
	}
	memset(hello_devices, 0, hello_nr_devs * sizeof(struct hello_dev));

        /* Initialize each device. */
	for (i = 0; i < hello_nr_devs; i++) {		
		hello_setup_cdev(&hello_devices[i], i);
	}


	printk("Hello: registered module successfully!\n");
	return 0;
      	  
 	hello_fail: 
		hello_exit(); /* Make this more graceful */
   		return ret;
   } 
 
 

  
 module_init(hello_init); 
 module_exit(hello_exit); 

 MODULE_AUTHOR("Himanshu Jain"); 
 MODULE_DESCRIPTION("Test Driver Example"); 
 MODULE_LICENSE("Dual BSD/GPL");
