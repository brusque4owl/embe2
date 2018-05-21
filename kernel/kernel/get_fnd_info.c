#include <linux/kernel.h>
#include <linux/uaccess.h>

#define FND_LENGTH 4
struct fnd_args {
	int time_interval, time_repeat;
	char start_option[5];
};

asmlinkage int sys_get_fnd_info(struct fnd_args *app_args){
	int i;
	int ret_val;
	int fnd_place, fnd_value, other;
	char *start_option;
	struct fnd_args temp_args;

	copy_from_user(&temp_args, app_args, sizeof(temp_args));
	start_option = temp_args.start_option;
	
	// Find the start place and start value
	for(i=0; i<FND_LENGTH; i++){
		if(start_option[i]!=0x30) break; // ASCII 0 = 0X30
	}
	
	// i means the place, start_option[i] means the value
	fnd_place = i;
	fnd_value = start_option[i] - 0x30;	// change to number
	/*
	printk("[syscall]       fnd_place = %d\n", fnd_place);
	printk("[syscall]       fnd_value = %d\n", fnd_value);
	printk("[syscall]   time_interval = %d\n", temp_args.time_interval);
	printk("[syscall]     time_repeat = %d\n", temp_args.time_repeat);
	*/

	// time_interval is upper 8bit of other, time_repeat is lower 8bit of other
	other = (temp_args.time_interval << 8) + temp_args.time_repeat;

	ret_val = (fnd_place << 24) + (fnd_value << 16) + other;
	/*
	printk("[syscall]   fnd_place<<24 = %d\n", fnd_place << 24);
	printk("[syscall]   fnd_value<<16 = %d\n", fnd_value << 16);
	printk("[syscall]time_interval<<8 = %d\n", temp_args.time_interval << 8);
	printk("[syscall]time_repeat      = %d\n", temp_args.time_repeat);
	printk("sys_get_fnd_info syscall is done\n");
	*/
	return ret_val;
}

