/*
Purpose:
Monitor a serial port and blink the DDK LED whenever any data is received.

Logic:
1. Open the serial port and configure baud rate.
2. Open the DDK device file.
3. Wait for serial data using select().
4. If data arrives, read it and blink the LED a few times.
5. Repeat forever.

Usage:
sudo ./serial_blink_led /dev/ttyUSB0 /dev/ddk_led0 9600

Notes:
- Replace /dev/ttyUSB0 with your actual serial port.
- Replace 9600 with your actual baud rate.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#include "ddk_mem.h"
#include "ddk.h"

#define BUF_SIZE 256

static speed_t get_baud(int baud)
{
    switch (baud)
    {
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        default: return B9600;
    }
}

static int setup_serial(const char *port, int baud)
{
    int fd = open(port, O_RDONLY | O_NOCTTY);
    if (fd < 0)
    {
        perror("open serial");
        return -1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    if (tcgetattr(fd, &tty) != 0)
    {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    cfsetispeed(&tty, get_baud(baud));
    cfsetospeed(&tty, get_baud(baud));

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= CREAD | CLOCAL;
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(ICRNL | INLCR);

    tty.c_lflag = 0;
    tty.c_oflag = 0;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        perror("tcsetattr");
        close(fd);
        return -1;
    }

    return fd;
}

static int set_led(int ddk_fd, int value)
{
    if (ioctl(ddk_fd, DDK_LED_SET, value) == -1)
    {
        perror("ioctl DDK_LED_SET");
        return -1;
    }
    return 0;
}

static int blink_led(int ddk_fd, int times, useconds_t delay_us)
{
    for (int i = 0; i < times; i++)
    {
        if (set_led(ddk_fd, 1) == -1)
            return -1;
        usleep(delay_us);

        if (set_led(ddk_fd, 0) == -1)
            return -1;
        usleep(delay_us);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <serial_port> <ddk_device> <baud>\n", argv[0]);
        fprintf(stderr, "Example: sudo %s /dev/ttyUSB0 /dev/ddk_led0 9600\n", argv[0]);
        return 1;
    }

    const char *serial_port = argv[1];
    const char *ddk_device = argv[2];
    int baud = atoi(argv[3]);

    int serial_fd = setup_serial(serial_port, baud);
    if (serial_fd < 0)
        return 1;

    int ddk_fd = open(ddk_device, O_RDWR);
    if (ddk_fd < 0)
    {
        perror("open ddk device");
        close(serial_fd);
        return 1;
    }

    printf("Listening on %s at %d baud...\n", serial_port, baud);
    printf("Blinking LED on %s whenever data is received.\n", ddk_device);

    char buf[BUF_SIZE];

    while (1)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(serial_fd, &readfds);

        int ret = select(serial_fd + 1, &readfds, NULL, NULL, NULL);
        if (ret < 0)
        {
            if (errno == EINTR)
                continue;
            perror("select");
            break;
        }

        if (FD_ISSET(serial_fd, &readfds))
        {
            ssize_t n = read(serial_fd, buf, sizeof(buf) - 1);
            if (n < 0)
            {
                perror("read serial");
                break;
            }

            if (n > 0)
            {
                buf[n] = '\0';
                printf("Received %zd bytes: %s\n", n, buf);

                if (blink_led(ddk_fd, 3, 200000) == -1)
                    break;
            }
        }
    }

    close(ddk_fd);
    close(serial_fd);
    return 0;
}
