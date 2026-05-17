#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "ddk_led.h"

#define READ_SIZE 80
#define WRITE_SIZE 80
#define BUF_SIZE (WRITE_SIZE + 1)

int main(int argc, char *argv[])
{
	char *filename;
	int fd;
	int choice;
	unsigned char data[BUF_SIZE];
	int i, cnt;
	int led_val = 1;

	if (argc != 2)
	{
		printf("Usage: %s <device_file_name>\n", argv[0]);
		return 1;
	}
	else
	{
		filename = argv[1];
	}

	fd = open(filename, O_RDWR);
	if (fd == -1)
	{
		perror("usb_ops open");
		return 1;
	}

	do
	{
		printf(" 0: Exit\n");
		printf(" 1: Get LED Status from DDK\n");
		printf(" 2: Set LED Status of DDK\n");
		printf("Enter choice: ");
		scanf("%d", &choice);
		getchar();
		switch (choice)
		{
			case 1:
				if (ioctl(fd, DDK_LED_GET, &led_val) == -1)
				{
					perror("usb_ops ioctl");
					break;
				}
				printf(" LED is %s\n", (led_val == 0) ? "Off" : "On");
				break;
			case 2:
				//led_val = !led_val;

				printf(" Setting LED to, OFF=0, ON=1 ... ");
				scanf("%d", &led_val);
				if (ioctl(fd, DDK_LED_SET, led_val) == -1)
				{
					perror("usb_ops ioctl");
					printf("failed\n");
					break;
				}
				printf("done\n");
				break;
		}
	} while (choice != 0);

	close (fd);
	return 0;
}
