/*
Purpose:
Continuously blink LED using ioctl calls to kernel driver.

Logic:
1. Open device file
2. Repeatedly set LED ON and OFF
3. Sleep between toggles
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include "ddk_mem.h"
#include "ddk.h"

int main(int argc, char *argv[])
{
    int fd;
    char *filename;
    int led_val;

    if (argc != 2)
    {
        printf("Usage: %s <device_file>\n", argv[0]);
        return 1;
    }

    filename = argv[1];

    fd = open(filename, O_RDWR);
    if (fd == -1)
    {
        perror("open");
        return 1;
    }

    printf("Starting LED blink...\n");

    while (1)
    {
        // LED ON
        led_val = 1;
        ioctl(fd, DDK_LED_SET, led_val);
        printf("LED ON\n");
        sleep(1);

        // LED OFF
        led_val = 0;
        ioctl(fd, DDK_LED_SET, led_val);
        printf("LED OFF\n");
        sleep(1);
    }

    close(fd);
    return 0;
}
