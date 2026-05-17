/* ddk_ext.h — Extension state for DDK USB driver
 * Include this in your existing driver .c file
 */
#ifndef DDK_EXT_H
#define DDK_EXT_H

#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define DDK_PROC_NAME   "ddk_usb_status"
#define DDK_TIMER_DEF_MS    1000    /* default timer interval */
#define DDK_BLINK_DEF_CNT   3
#define DDK_BLINK_DEF_MS    200

struct ddk_ext_state {
    /* runtime stats */
    unsigned long   total_usb_success;
    unsigned long   total_usb_fail;
    unsigned long   total_blinks;
    unsigned int    last_write_len;

    /* timer config */
    bool            timer_enabled;
    unsigned long   timer_interval_ms;
    struct timer_list activity_timer;

    /* blink config */
    int             blink_count;
    int             blink_delay_ms;

    /* workqueue */
    struct work_struct  blink_work;
    bool            work_pending;

    /* proc entry */
    struct proc_dir_entry *proc_entry;

    /* lock — protects all fields above */
    spinlock_t      lock;
};

/* called from driver init/exit */
int  ddk_ext_init(struct ddk_ext_state *ext);
void ddk_ext_cleanup(struct ddk_ext_state *ext);

/* called from USB write path */
void ddk_ext_on_write_success(struct ddk_ext_state *ext, size_t len);
void ddk_ext_on_write_fail(struct ddk_ext_state *ext);

#endif /* DDK_EXT_H */
