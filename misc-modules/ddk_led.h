#ifndef DDK_LED_H
#define DDK_LED_H

#include <linux/ioctl.h>

#define DDK_LED_GET _IOR('u', 1, int *)
#define DDK_LED_SET _IOW('u', 2, int)
#define DDK_WR_OFF_SET _IOW('u', 3, int *)
#define DDK_RD_OFF_SET _IOW('u', 4, int *)
#define DDK_RD_OFF_GET _IOR('u', 5, int *)
#define DDK_WR_OFF_GET _IOR('u', 6, int *)
#define DDK_MEM_SIZE_GET _IOR('u', 7, int *)
#endif
