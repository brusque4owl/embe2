#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>

#include "dev_driver.h"

#define DEV_DRIVER_MAJOR 242
#define DEV_DRIVER_MINOR 0
#define DEV_DRIVER_NAME "dev_driver"

// Addresses of fpga_modules
#define IOM_FND_ADDRESS 0x08000004	// address of fnd
#define IOM_LED_ADDRESS 0x08000016	// address of led
#define IOM_DOT_ADDRESS 0x08000210	// address of dot
#define IOM_LCD_ADDRESS 0x08000090	// address of lcd(16*2=32bytes)

// Global variable for fpga_modules
static int dev_driver_usage = 0;
static unsigned char *iom_fpga_fnd_addr;

// Define functions
int dev_driver_open(struct inode *, struct file *);
int dev_driver_release(struct inode *, struct file *);
ssize_t dev_driver_write(struct file *, const char *, size_t, loff_t *);
long dev_driver_ioctl(struct file *inode,
					 unsigned int ioctl_num,
					 unsigned long ioctl_param);

// Define file_operations structure
static struct file_operations dev_driver_fops =
{ 
	.owner 			= THIS_MODULE,
	.open 			= dev_driver_open, 
	.write 			= dev_driver_write,
	.unlocked_ioctl = dev_driver_ioctl,
	.release 		= dev_driver_release
};

static struct struct_mydata {
	struct timer_list timer;
	int count;
	char fnd_place;
	char fnd_value;
	char time_interval;
	char time_repeat;
};
struct struct_mydata mydata;	// struct var for timer

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
	int i;
	struct struct_mydata *p_data = (struct struct_mydata*)timeout;
	// Variables for FND module
	unsigned short int value_short = 0;
	unsigned char value[4];

	// Check for terminating timer
	p_data->count++;
	if( p_data->count > p_data->time_repeat ) {
		return;
	}
	printk("Executed kernel_timer_count %d\n", p_data->count);

	// Write to fpga modules(fnd, led, dot, lcd)
	// 1. Write to fnd device
	for(i=0;i<4;i++){ // resolve 4bytes stream on fnd
		if(p_data->fnd_place==i){
			if(p_data->fnd_value%8==0)
				value[i]=8;
			else
				value[i]=p_data->fnd_value%8;
		}
		else
			value[i]=0;
	}
	value_short = value[0] << 12 | value[1] << 8 |
				  value[2] << 4  | value[3];
	outw(value_short, (unsigned int)iom_fpga_fnd_addr);
		// update fnd_place, fnd_value;
	p_data->fnd_value++;
	if(p_data->count%8 == 0){
		p_data->fnd_place++;
		if(p_data->fnd_place>3) p_data->fnd_place=0;
	}
	// 2. Write to led device

	// 3. Write to dot device

	// 4. Write to lcd device

	// re-register timer
	mydata.timer.expires=get_jiffies_64()+(p_data->time_interval*HZ/10);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function	= kernel_timer_blink;

	add_timer(&mydata.timer);
}

ssize_t dev_driver_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	int i;
	const char *tmp = gdata;// gdata is syscall ret val
	char syscall_data[4];	// store return value of syscall
	char fnd_place, fnd_value, time_interval, time_repeat;

	printk("dev_driver_write\n");
	if (copy_from_user(&syscall_data, tmp, 1)) {
		return -EFAULT;
	}
	printk("\n\n");
	for(i=0;i<4;i++){
		printk("[dev_write] gdata[%d] = %d\n", i, gdata[i]);
	}
	printk("\n\n");
	fnd_place = gdata[0];
	fnd_value = gdata[1];
	time_interval = gdata[2];
	time_repeat   = gdata[3];

	// Start of Setting timer - don't insert anything in it.
	mydata.count = 0;
	mydata.fnd_place = fnd_place;
	mydata.fnd_value = fnd_value;
	mydata.time_interval = time_interval;
	mydata.time_repeat = time_repeat;
	printk("timer repeat : %d \n",mydata.time_repeat);

	del_timer_sync(&mydata.timer);

	mydata.timer.expires=get_jiffies_64()+(time_interval*HZ/10);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function	= kernel_timer_blink;

	add_timer(&mydata.timer);
	// End of setting timer

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

	// Mapping fpga physical mem to kernel(fnd, led, dot, lcd)
	iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);

	// Initialize timer
	init_timer(&(mydata.timer));

	printk("init module\n");
	return 0;
}

void __exit dev_driver_exit(void)
{
	printk("dev_driver_exit\n");
	dev_driver_usage = 0;
	del_timer_sync(&mydata.timer);

	// Unmappingg fpga physical mem from kernel
	iounmap(iom_fpga_fnd_addr);

	unregister_chrdev(DEV_DRIVER_MAJOR, DEV_DRIVER_NAME);
}

module_init( dev_driver_init);
module_exit( dev_driver_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("author");
