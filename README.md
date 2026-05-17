# Linux USB Device Driver with Proc, Timer and Workqueue

---

## Project Overview

This project extends a Linux USB device driver for the **eSrijan LDDK board** by integrating advanced kernel mechanisms including the proc filesystem interface, kernel timers, workqueues, and delay handling. It enables interaction with the device's EEPROM memory and programmable LED via USB, and introduces a **fault injection framework** for testing driver robustness.

A user-space application is developed to communicate with the kernel module using system calls such as `ioctl`, `read`, and `write`.

---

## Objectives

- Implement a **proc filesystem interface** (`/proc/ddk_usb_status`) for bidirectional user-kernel communication
- Use **kernel timers** (`timer_setup`, `mod_timer`) for periodic execution and monitoring
- Implement **workqueues** (`INIT_WORK`, `schedule_work`) for asynchronous task processing
- Incorporate **delay mechanisms** (`msleep`) in kernel space for LED control
- Modify an existing USB driver to integrate all the above mechanisms
- Design a **user-space testing tool** for driver validation

---

## System Components

### 1. Kernel Module (Modified Driver)
The core driver (`ddk_mem.c`) is modified in the `ddk_write()` function to:
- Track success/failure counts after each `usb_interrupt_msg()` call
- Arm the kernel timer on successful USB write
- Integrate with the workqueue mechanism

### 2. Proc Interface (`/proc/ddk_usb_status`)
- **Read:** `cat /proc/ddk_usb_status` — displays runtime statistics (USB success/failure counts, LED blink count, timer and workqueue status)
- **Write:** `echo "timer 1" > /proc/ddk_usb_status` — dynamically configures driver behavior

### 3. Kernel Timer
- Initialized with `timer_setup()`
- Armed after every successful USB write using `mod_timer()`
- Callback (`activity_timer_cb`) re-arms itself every ~1 second
- Schedules deferred work via `schedule_work()` (since timer runs in atomic context)

### 4. Workqueue
- Work item initialized with `INIT_WORK()`, linked to `blink_worker()`
- Runs in **process context**, allowing safe use of sleeping operations
- Handles LED blinking, blink counter updates, and other deferred tasks

### 5. Delay Mechanism
- `msleep()` used inside workqueue worker for LED ON/OFF toggle timing
- **Not** used inside timer callbacks (atomic context restriction)

### 6. User-Space Application
A menu-driven application (`ddk_app.c`) providing:
- LED control via `ioctl()`
- EEPROM read/write via `read()` / `write()`
- USB data transmission
- Driver status monitoring via `/proc`
- Dynamic driver configuration via `/proc`

---

## Architecture

```
User Space
  │
  ├── ddk_app  ──────────────────────────┐
  │   (ioctl / read / write / /proc)     │
  │                                      ▼
Kernel Space                     /proc/ddk_usb_status
  │
  ├── ddk_mem.c  (USB File Ops: open, read, write, ioctl)
  │       │
  │       ├── On write success ──► mod_timer()
  │       └── On write failure ──► increment fail_count
  │
  ├── Kernel Timer (activity_timer_cb)
  │       └── schedule_work() ──► Workqueue
  │
  └── Workqueue (blink_worker)
          ├── LED Blink (msleep)
          └── Update blink_count
```

---

## Build Instructions

### Prerequisites
- Linux kernel headers installed
- GCC toolchain
- LDDK board / eSrijan USB device

### Building the Kernel Module

```bash
cd kernel/
make
```

### Loading the Module

```bash
sudo insmod ddk_ext.ko
sudo insmod ddk_mem.ko
```

### Verifying the Module

```bash
lsmod | grep ddk
dmesg | tail -20
cat /proc/ddk_usb_status
```

### Building the User-Space Application

```bash
cd userspace/
make
```

### Running the Application

```bash
sudo ./ddk_app
```

### Removing the Module

```bash
sudo rmmod ddk_mem
sudo rmmod ddk_ext
```

---

## Fault Injection

The driver includes a fault injection framework to simulate USB write failures:
- Triggers `ddk_ext_on_write_fail()` on failure, incrementing the failure counter
- Allows testing of error handling and system reliability
- Fault behavior can be observed via `/proc/ddk_usb_status`

---

## Key Observations

- USB driver uses separate TX and RX buffers
- Read data ≠ written data (no direct USB loopback)
- Validation is based on `write()` return value
- Fault injection effectively tests error handling paths

---

## Advantages

- Demonstrates real-world Linux driver development concepts
- Implements safe asynchronous processing using workqueues
- Provides a live debugging/monitoring interface via `/proc`
- Modular and easy to extend

## Limitations

- No direct USB loopback verification
- Limited hardware (single programmable LED on LDDK board)
- Simplified fault injection model

---

## References

- [Linux Kernel Documentation — procfs](https://www.kernel.org/doc/html/latest/filesystems/proc.html)
- [Linux Kernel Documentation — Timers](https://www.kernel.org/doc/html/latest/timers/timers-howto.html)
- [Linux Kernel Documentation — Workqueues](https://www.kernel.org/doc/html/latest/core-api/workqueue.html)
- *Linux Device Drivers, 3rd Edition* — Corbet, Rubini, Kroah-Hartman
- eSrijan LDDK Board Reference Manual
