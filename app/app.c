#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "./fpga_dot_font.h"

#define FPGA_DOT_DEVICE "/dev/fpga_dot"
#define FND_DEVICE "/dev/fpga_fnd"
#define LED_DEVICE "/dev/fpga_led"
#define FPGA_TEXT_LCD_DEVICE "/dev/fpga_text_lcd"

#define MAX_DIGIT 4	// MAX value for fnd_device
#define MAX_BUFF 32	// MAX value for lcd_device
#define LINE_BUFF 16// ecah line has 16 characters

struct fnd_args {
	int time_interval, time_repeat;
	char start_option[5];
};

int main(int argc, char **argv)
{
	int i;
	struct fnd_args input;		// store input arguments
	int num_start_option;		// check validity for 3rd arg
	int count_not_zero=0;		// check validity for 3rd arg
	int ret_val;				// return value for syscall
	// check for number of arguments
	if(argc!=4) {
		printf("please input 3 parameters.\n");
		printf("ex) ./app 1 14 0700\n");
		return -1;
	}
	
	input.time_interval = atoi(argv[1]);
	input.time_repeat   = atoi(argv[2]);
	strcpy(input.start_option, argv[3]);
	num_start_option = atoi(argv[3]);

	// check for the 1st argument
	if(input.time_interval<1 || input.time_interval>100){
		printf("[1st arg]The range of time interval is from 1 to 100\n");
		return -1;
	}
	// check for the 2nd argument
	if(input.time_repeat<1 || input.time_repeat>100){
		printf("[2nd arg]The range of time repeat is from 1 to 100\n");
		return -1;
	}
	// check for the 3rd argument - 1) length
	if(strlen(input.start_option)!=4){
		printf("[3rd arg]start option must have 4-length string\n");
		printf("         The range is from 0001 to 8000\n");
		return -1;
	}
	// check for the 3rd argument - 2) number of not zero
	//								   except 9
	for(i=0;i<4;i++){
		if(input.start_option[i]!=0x30)	// ASCII 0 = 0x30
			count_not_zero++;
		if(input.start_option[i]==0x39){
			printf("[3rd arg]start option can't have 9 in the value\n");
			return -1;
		}
	}
	if(count_not_zero!=1){
		printf("[3rd arg]start option must have three zero\n");
		return -1;
	}
	// check for the 3rd argument - 3) range
	if(num_start_option<1 || num_start_option>8000){
		printf("[3rd arg]The range of start option is from 0001 to 8000\n");
		return -1;
	}

	// Implement main program
	ret_val = syscall(376, &input);
	printf("syscall ret_val = %d\n", ret_val);

	printf("Success.\n");
	return 0;
}
