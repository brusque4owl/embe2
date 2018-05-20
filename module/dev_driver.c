#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

#define DEV_DRIVER_MAJOR 242
#define DEV_DRIVER_MINOR 0
#define DEV_DRIVER_NAME "dev_driver"

static int dev_driver_usage = 0;

int dev_driver_open(struct inode *, struct file *);
int dev_driver_release(struct inode *, struct file *);
ssize_t dev_driver_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations dev_driver_fops =
{ 
	.open = dev_driver_open, 
	.write = dev_driver_write,
	.release = dev_driver_release
};

static struct struct_mydata {
	struct timer_list timer;
	int count;
};

struct struct_mydata mydata;

int dev_driver_release(struct inode *minode, struct file *mfile) {
	printk("dev_driver_release\n");
	dev_driver_usage = 0;
	return 0;
}

int dev_driver_open(struct inode *minode, struct file *mfile) {
	printk("dev_driver_open\n");
	if (dev_driver_usage != 0) {
		return -EBUSY;
	}
	dev_driver_usage = 1;
	return 0;
}

static void kernel_timer_blink(unsigned long timeout) {
	struct struct_mydata *p_data = (struct struct_mydata*)timeout;

	printk("kernel_timer_blink %d\n", p_data->count);

	p_data->count++;
	if( p_data->count > 15 ) {
		return;
	}

	mydata.timer.expires = get_jiffies_64() + (1 * HZ);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function = kernel_timer_blink;

	add_timer(&mydata.timer);
}

ssize_t dev_driver_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	const char *tmp = gdata;
	char syscall_data[4];	// store return value of syscall
	printk("dev_driver_write\n");
	if (copy_from_user(&syscall_data, tmp, 1)) {
		return -EFAULT;
	}
	int i;
	printk("\n\n");
	for(i=0;i<4;i++){
		printk("gdata[%d] = %d\n", i, gdata[i]);
	}
	printk("\n\n");
	//char fnd_place, fnd_value, time_interval, time_repeat;
	/*
	mydata.count = kernel_timer_buff;

	printk("data  : %d \n",mydata.count);

	del_timer_sync(&mydata.timer);

	mydata.timer.expires = jiffies + (1 * HZ);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function	= kernel_timer_blink;

	add_timer(&mydata.timer);
	*/
	return 1;
}

int __init dev_driver_init(void)
{
	int result;
	printk("dev_driver_init\n");

	result = register_chrdev(DEV_DRIVER_MAJOR, DEV_DRIVER_NAME, &dev_driver_fops);
	if(result <0) {
		printk( "error %d\n",result);
		return result;
	}
    printk( "dev_file : /dev/%s , major : %d\n",DEV_DRIVER_NAME, DEV_DRIVER_MAJOR);

	init_timer(&(mydata.timer));

	printk("init module\n");
	return 0;
}

void __exit dev_driver_exit(void)
{
	printk("dev_driver_exit\n");
	dev_driver_usage = 0;
	del_timer_sync(&mydata.timer);

	unregister_chrdev(DEV_DRIVER_MAJOR, DEV_DRIVER_NAME);
}

module_init( dev_driver_init);
module_exit( dev_driver_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("author");
