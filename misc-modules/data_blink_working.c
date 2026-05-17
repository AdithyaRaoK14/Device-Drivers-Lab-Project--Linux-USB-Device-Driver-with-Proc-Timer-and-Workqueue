/*
Purpose:
Use the exact working DDK userspace pattern from the original sample,
but simplify it so that every message written to the device causes the LED to blink.

Logic:
1. Open /dev/ddk_led0
2. Read one line of user input
3. Write exactly WRITE_SIZE bytes to the device
4. Blink LED after successful write
5. Repeat until user enters exit
*/

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include "ddk_mem.h"
#include "ddk.h"

#define WRITE_SIZE 80
#define BUF_SIZE (WRITE_SIZE + 1)

static int blink_led(int fd, int times, useconds_t delay_us)
{
    int led_val;
    int i;

    for (i = 0; i < times; i++)
    {
        led_val = 1;
        if (ioctl(fd, DDK_LED_SET, led_val) == -1)
        {
            perror("usb_ops ioctl LED ON");
            return -1;
        }
        usleep(delay_us);

        led_val = 0;
        if (ioctl(fd, DDK_LED_SET, led_val) == -1)
        {
            perror("usb_ops ioctl LED OFF");
            return -1;
        }
        usleep(delay_us);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    char *filename;
    int fd;
    char *data;

    data = (char *)malloc(sizeof(char) * BUF_SIZE);
    if (data == NULL)
    {
        perror("malloc");
        return 1;
    }

    if (argc != 2)
    {
        printf("Usage: %s <device_file_name>\n", argv[0]);
        free(data);
        return 1;
    }

    filename = argv[1];

    fd = open(filename, O_RDWR);
    if (fd == -1)
    {
        perror("usb_ops open");
        free(data);
        return 1;
    }

    printf("Type a message and press Enter.\n");
    printf("LED will blink whenever data is written.\n");
    printf("Type exit to quit.\n");

    while (1)
    {
        memset(data, 0, BUF_SIZE);

        printf("\nEnter data: ");
        fflush(stdout);

        if (scanf("%[^\n]", data) != 1)
        {
            int ch = getchar();
            if (ch == EOF)
                break;
            continue;
        }

        getchar();  /* consume newline */

        if (strcmp(data, "exit") == 0)
            break;

        if (write(fd, data, WRITE_SIZE) == -1)
        {
            perror("write");
            close(fd);
            free(data);
            return 1;
        }

        printf("Data written\n");

        if (blink_led(fd, 3, 200000) == -1)
        {
            close(fd);
            free(data);
            return 1;
        }

        printf("Blink done\n");
    }

    close(fd);
    free(data);
    return 0;
}
