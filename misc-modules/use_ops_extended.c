#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "ddk_mem.h"
#include "ddk.h"

#define READ_SIZE 80
#define WRITE_SIZE 80
#define BUF_SIZE (WRITE_SIZE + 1)

/* Blink LED `times` times, with `delay_us` between toggles */
int blink_led(int fd_led, int times, useconds_t delay_us)
{
    for (int i = 0; i < times; i++) {
        if (ioctl(fd_led, DDK_LED_SET, 1) == -1) {
            perror("blink_led ON");
            return -1;
        }
        usleep(delay_us);
        if (ioctl(fd_led, DDK_LED_SET, 0) == -1) {
            perror("blink_led OFF");
            return -1;
        }
        usleep(delay_us);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int fd_mem, fd_led;
    int choice;
    char *data;
    int led_val = 1;
    int offset_val;
    int size_val;

    data = (char *)malloc(sizeof(char) * BUF_SIZE);
    if (!data) {
        perror("malloc");
        return 1;
    }

    if (argc != 3) {
        printf("Usage: %s <mem_device> <led_device>\n", argv[0]);
        printf("  e.g. %s /dev/ddk_mem1 /dev/ddk_led0\n", argv[0]);
        free(data);
        return 1;
    }

    fd_mem = open(argv[1], O_RDWR);
    if (fd_mem == -1) {
        perror("open mem device");
        free(data);
        return 1;
    }

    fd_led = open(argv[2], O_RDWR);
    if (fd_led == -1) {
        perror("open led device");
        close(fd_mem);
        free(data);
        return 1;
    }

    do {
        printf("\n  0: Exit\n");
        printf("  1: Get LED Status\n");
        printf("  2: Set LED Status\n");
        printf("  3: Set write offset\n");
        printf("  4: Set read offset\n");
        printf("  5: Get read offset\n");
        printf("  6: Get write offset\n");
        printf("  7: Get memory size\n");
        printf("  8: Read from memory\n");
        printf("  9: Write to memory (blinks LED on success)\n");
        printf(" 10: Send data via USB (best-effort confirm, blinks LED on success)\n");
        printf(" 11: Show /proc/ddk_usb_status\n");
        printf(" 12: Send proc command\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                if (ioctl(fd_led, DDK_LED_GET, &led_val) == -1) {
                    perror("ioctl LED_GET");
                    break;
                }
                printf(" LED is %s\n", (led_val == 0) ? "Off" : "On");
                break;

            case 2:
                printf(" Setting LED (OFF=0, ON=1): ");
                scanf("%d", &led_val);
                getchar();
                if (ioctl(fd_led, DDK_LED_SET, led_val) == -1) {
                    perror("ioctl LED_SET");
                    break;
                }
                printf("done\n");
                break;

            case 3:
                printf(" Setting write offset: ");
                scanf("%d", &offset_val);
                getchar();
                if (ioctl(fd_mem, DDK_WR_OFF_SET, offset_val) == -1) {
                    perror("ioctl WR_OFF_SET");
                    break;
                }
                printf("done\n");
                break;

            case 4:
                printf(" Setting read offset: ");
                scanf("%d", &offset_val);
                getchar();
                if (ioctl(fd_mem, DDK_RD_OFF_SET, offset_val) == -1) {
                    perror("ioctl RD_OFF_SET");
                    break;
                }
                printf("done\n");
                break;

            case 5:
                if (ioctl(fd_mem, DDK_RD_OFF_GET, &offset_val) == -1) {
                    perror("ioctl RD_OFF_GET");
                    break;
                }
                printf(" Read offset is: %d\n", offset_val);
                break;

            case 6:
                if (ioctl(fd_mem, DDK_WR_OFF_GET, &offset_val) == -1) {
                    perror("ioctl WR_OFF_GET");
                    break;
                }
                printf(" Write offset is: %d\n", offset_val);
                break;

            case 7:
                if (ioctl(fd_mem, DDK_MEM_SIZE_GET, &size_val) == -1) {
                    perror("ioctl MEM_SIZE_GET");
                    break;
                }
                printf(" Memory size is %d\n", size_val);
                break;

            case 8: {
                memset(data, 0, BUF_SIZE);
                int n = read(fd_mem, data, READ_SIZE);
                if (n == -1) {
                    perror("read");
                    break;
                }
                printf(" Read %d bytes: ", n);
                for (int i = 0; i < n; i++) printf("%c", data[i]);
                printf("\n");
                break;
            }

            case 9: {
                memset(data, 0, BUF_SIZE);
                printf("Enter data to write: ");
                if (scanf("%80[^\n]", data) != 1) {
                    printf("No input given\n");
                    getchar();
                    break;
                }
                getchar();

                int len = strlen(data);
                int wr_off = 0;
                ioctl(fd_mem, DDK_WR_OFF_SET, wr_off);

                if (write(fd_mem, data, len) == -1) {
                    perror("write");
                    break;
                }
                printf("Data written (%d bytes)\n", len);

                if (blink_led(fd_led, 3, 200000) == -1) {
                    printf("LED blink failed\n");
                    break;
                }
                printf("LED blink done\n");
                break;
            }

            /*
             * Option 10: USB send with best-effort confirmation.
             *
             * The driver exposes independent TX and RX buffers — read() does
             * not reflect what was written (confirmed by hex inspection: read
             * returns unrelated bytes from the RX buffer, not a loopback).
             * True write+readback comparison is therefore not possible with
             * this driver interface.
             *
             * Best-effort confirmation strategy used here:
             *   a) Reset write offset to 0
             *   b) Write the data via the driver's USB TX path
             *   c) Confirm that write() returned exactly len bytes (no short write)
             *   d) Blink LED only if the full transfer was accepted by the driver
             *   e) If write() returns fewer bytes or -1, report failure and do not blink
             */
            case 10: {
                memset(data, 0, BUF_SIZE);

                printf("Enter data to send via USB: ");
                if (scanf("%80[^\n]", data) != 1) {
                    printf("No input given\n");
                    getchar();
                    break;
                }
                getchar();

                int len = strlen(data);

                /* Step a: reset write offset before sending */
                if (ioctl(fd_mem, DDK_WR_OFF_SET, 0) == -1) {
                    perror("ioctl WR_OFF_SET (USB send)");
                    break;
                }

                /* Step b: send data through the driver's USB TX path */
                int n_wr = write(fd_mem, data, len);
                if (n_wr == -1) {
                    perror("write (USB send)");
                    printf("USB send FAILED — LED will not blink.\n");
                    break;
                }

                /* Step c/d/e: confirm all bytes were accepted */
                if (n_wr == len) {
                    printf("USB send confirmed: %d of %d bytes accepted by driver.\n",
                           n_wr, len);
                    if (blink_led(fd_led, 3, 200000) == -1) {
                        printf("LED blink failed\n");
                        break;
                    }
                    printf("LED blink done\n");
                } else {
                    printf("USB send incomplete: only %d of %d bytes accepted.\n",
                           n_wr, len);
                    printf("LED will NOT blink — transfer not fully confirmed.\n");
                }
                break;
            }
        
        case 11: {
                FILE *fp = fopen("/proc/ddk_usb_status", "r");
                if (!fp) { perror("open proc"); break; }
                char line[128];
                while (fgets(line, sizeof(line), fp))
                    printf("%s", line);
                fclose(fp);
                break;
            }

            case 12: {
                char cmd[64];
                printf("Enter proc command (e.g. 'timer 1', 'blink 5', 'reset'): ");
                if (scanf(" %63[^\n]", cmd) != 1) { getchar(); break; }
                getchar();
                FILE *fp = fopen("/proc/ddk_usb_status", "w");
                if (!fp) { perror("open proc write"); break; }
                fprintf(fp, "%s\n", cmd);
                fclose(fp);
                printf("Command sent: %s\n", cmd);
                break;
            }
            }
    } while (choice != 0);

    close(fd_mem);
    close(fd_led);
    free(data);
    return 0;
}
