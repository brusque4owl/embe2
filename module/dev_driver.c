#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

#include "dev_driver.h"

#define DEV_DRIVER_MAJOR 242
#define DEV_DRIVER_MINOR 0
#define DEV_DRIVER_NAME "dev_driver"

static int dev_driver_usage = 0;

int dev_driver_open(struct inode *, struct file *);
int dev_driver_release(struct inode *, struct file *);
ssize_t dev_driver_write(struct file *, const char *, size_t, loff_t *);
long dev_driver_ioctl(struct file *inode,
					 unsigned int ioctl_num,
					 unsigned long ioctl_param);

static struct file_operations dev_driver_fops =
{ 
	.open = dev_driver_open, 
	.write = dev_driver_write,
	.unlocked_ioctl = dev_driver_ioctl,
	.release = dev_driver_release
};

static struct struct_mydata {
	struct timer_list timer;
	int count;
	char time_interval;
	char time_repeat;
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

	printk("kernel_timer_count %d\n", p_data->count);

	p_data->count++;
	if( p_data->count > p_data->time_repeat ) {
		return;
	}
	mydata.timer.expires=get_jiffies_64()+(p_data->time_interval*HZ/10);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function	= kernel_timer_blink;

	add_timer(&mydata.timer);
}

ssize_t dev_driver_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	int i;
	const char *tmp = gdata;
	char syscall_data[4];	// store return value of syscall
	char fnd_place, fnd_value, time_interval, time_repeat;

	printk("dev_driver_write\n");
	if (copy_from_user(&syscall_data, tmp, 1)) {
		return -EFAULT;
	}
	printk("\n\n");
	for(i=0;i<4;i++){
		printk("gdata[%d] = %d\n", i, gdata[i]);
	}
	printk("\n\n");
	fnd_place = gdata[0];
	fnd_value = gdata[1];
	time_interval = gdata[2];
	time_repeat   = gdata[3];

//*******************************************************
// Start of Setting timer
	mydata.count = 0;
	mydata.time_interval = time_interval;
	mydata.time_repeat = time_repeat;
	printk("timer repeat : %d \n",mydata.time_repeat);

	del_timer_sync(&mydata.timer);

	mydata.timer.expires=get_jiffies_64()+(time_interval*HZ/10);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function	= kernel_timer_blink;

	add_timer(&mydata.timer);
// End of setting timer
//*******************************************************
	return 1;
}

long dev_driver_ioctl(struct file *inode,
					 unsigned int ioctl_num,
					 unsigned long ioctl_param) {
	int ret_val;
	printk("\n\nnow it is using ioctl in kernel\n\n");	
	switch(ioctl_num){
		case IOCTL_WRITE:
			ret_val = dev_driver_write(inode, (char *)ioctl_param, 0, 0);
			if(ret_val==-EFAULT) return -1;
			break;
	}
	return 0;
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
