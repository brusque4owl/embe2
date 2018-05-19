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

int main(int argc, char **argv)
{
	//      [1-100]       [1-100]    [0001-8000]
	int time_interval, time_repeat, start_option;

	// check for number of arguments
	if(argc!=4) {
		printf("please input 3 parameters.\n");
		printf("ex) ./app 1 14 0700\n");
		return -1;
	}
	
	time_interval = atoi(argv[1]);
	time_repeat   = atoi(argv[2]);
	start_option  = atoi(argv[3]);


	// check for the 1st argument
	if(time_interval<1 || time_interval>100){
		printf("The range of time interval is from 1 to 100\n");
		return -1;
	}
	// check for the 2nd argument
	if(time_repeat<1 || time_repeat>100){
		printf("The range of time repeat is from 1 to 100\n");
		return -1;
	}
	// check for the 3rd argument
	if(start_option<1 || start_option>8000){
		printf("The range of start option is from 0001 to 8000\n");
		return -1;
	}
	printf("Success.\n");
	return 0;
}
