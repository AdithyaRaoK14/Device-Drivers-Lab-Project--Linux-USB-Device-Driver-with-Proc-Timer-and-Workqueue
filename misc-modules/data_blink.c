/*
Purpose:
Write user-entered data to the DDK device and blink LED after successful write.

Logic:
1. Open /dev/ddk_led0
2. Read input using the same style as the original working program
3. Write exactly 80 bytes to the device
4. Blink LED if write succeeds
5. Repeat until user types exit
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ddk_mem.h"
#include "ddk.h"

#define WRITE_SIZE 80
#define BUF_SIZE (WRITE_SIZE + 1)

static int set_led(int fd, int value)
{
    if (ioctl(fd, DDK_LED_SET, value) == -1)
    {
        perror("ioctl DDK_LED_SET");
        return -1;
    }
    return 0;
}

static int blink_led(int fd, int times, useconds_t delay_us)
{
    for (int i = 0; i < times; i++)
    {
        if (set_led(fd, 1) == -1)
            return -1;
        usleep(delay_us);

        if (set_led(fd, 0) == -1)
            return -1;
        usleep(delay_us);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int fd;
    char *filename;
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
        perror("open");
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

        if (scanf("%80[^\n]", data) != 1)
        {
            int ch = getchar();
            if (ch == EOF)
                break;
            continue;
        }
        getchar();  // consume newline

        if (strcmp(data, "exit") == 0)
            break;

        if (write(fd, data, WRITE_SIZE) == -1)
        {
            perror("write");
            close(fd);
            free(data);
            return 1;
        }

        printf("Data written successfully: %s\n", data);

        if (blink_led(fd, 3, 200000) == -1)
        {
            close(fd);
            free(data);
            return 1;
        }
    }

    close(fd);
    free(data);
    return 0;
}
