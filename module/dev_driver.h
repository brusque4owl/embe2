#ifndef DEV_DRIVER_H
#define DEV_DRIVER_H

#include <asm/ioctl.h>

#define MAJOR_NUM 242

#define IOCTL_WRITE _IOR(MAJOR_NUM, 0, char *)


#endif
