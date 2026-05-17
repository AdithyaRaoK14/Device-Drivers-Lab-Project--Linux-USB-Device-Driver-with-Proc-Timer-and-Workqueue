/* ddk_usb_ext.c
 * Extension module: proc entry, timer, workqueue, delays
 * for the DDK USB driver mini project.
 *
 * ASSUMPTION: LED driver exports ddk_led_ctrl(int val).
 *             Add EXPORT_SYMBOL(ddk_led_ctrl) to your LED driver.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include "ddk_ext.h"

/* LED control symbol exported by the LED driver.
 * Prototype must match the definition in ddk_led.c.
 * ASSUMPTION: int ddk_led_ctrl(int val) exists and is exported.
 */
extern int ddk_led_ctrl(int val);

/* ------------------------------------------------------------------ */
/* Forward declarations                                                 */
/* ------------------------------------------------------------------ */
static void blink_worker(struct work_struct *work);
static void activity_timer_cb(struct timer_list *t);

/* ------------------------------------------------------------------ */
/* Workqueue worker — runs in process context, may sleep               */
/* ------------------------------------------------------------------ */
static void blink_worker(struct work_struct *work)
{
    struct ddk_ext_state *ext =
        container_of(work, struct ddk_ext_state, blink_work);
    int i, count, delay_ms;
    unsigned long flags;

    /* Snapshot config under lock, then release before sleeping */
    spin_lock_irqsave(&ext->lock, flags);
    count    = ext->blink_count;
    delay_ms = ext->blink_delay_ms;
    ext->work_pending = false;
    spin_unlock_irqrestore(&ext->lock, flags);

    pr_info("ddk_ext: blink_worker start — %d blinks @ %d ms\n",
            count, delay_ms);

    for (i = 0; i < count; i++) {
        ddk_led_ctrl(1);
        msleep(delay_ms);       /* safe: process context, may sleep */
        ddk_led_ctrl(0);
        msleep(delay_ms);
    }

    /* Update blink stat under lock */
    spin_lock_irqsave(&ext->lock, flags);
    ext->total_blinks += count;
    spin_unlock_irqrestore(&ext->lock, flags);

    pr_info("ddk_ext: blink_worker done — total_blinks=%lu\n",
            ext->total_blinks);
}

/* ------------------------------------------------------------------ */
/* Timer callback — atomic context, must NOT sleep                     */
/* Only queues work and optionally re-arms.                            */
/* ------------------------------------------------------------------ */
static void activity_timer_cb(struct timer_list *t)
{
    struct ddk_ext_state *ext = timer_container_of(ext, t, activity_timer);
    unsigned long flags;
    bool enabled;

    spin_lock_irqsave(&ext->lock, flags);
    enabled = ext->timer_enabled;
    if (!ext->work_pending) {
        ext->work_pending = true;
        schedule_work(&ext->blink_work);  /* non-sleeping, safe here */
    }
    spin_unlock_irqrestore(&ext->lock, flags);

    /* Re-arm timer for periodic mode if still enabled */
    if (enabled) {
        unsigned long interval;
        spin_lock_irqsave(&ext->lock, flags);
        interval = msecs_to_jiffies(ext->timer_interval_ms);
        spin_unlock_irqrestore(&ext->lock, flags);
        mod_timer(&ext->activity_timer, jiffies + interval);
    }
}

/* ------------------------------------------------------------------ */
/* Proc read — seq_file based                                          */
/* ------------------------------------------------------------------ */
static int ddk_proc_show(struct seq_file *m, void *v)
{
    struct ddk_ext_state *ext = m->private;
    unsigned long flags;

    /* Snapshot all fields under lock */
    unsigned long s, f, b;
    unsigned int  lwl;
    bool          te;
    unsigned long ti;
    int           bc, bd;
    bool          wp;

    spin_lock_irqsave(&ext->lock, flags);
    s   = ext->total_usb_success;
    f   = ext->total_usb_fail;
    b   = ext->total_blinks;
    lwl = ext->last_write_len;
    te  = ext->timer_enabled;
    ti  = ext->timer_interval_ms;
    bc  = ext->blink_count;
    bd  = ext->blink_delay_ms;
    wp  = ext->work_pending;
    spin_unlock_irqrestore(&ext->lock, flags);

    seq_printf(m, "DDK USB Driver — Runtime Status\n");
    seq_printf(m, "--------------------------------\n");
    seq_printf(m, "total_usb_success : %lu\n", s);
    seq_printf(m, "total_usb_fail    : %lu\n", f);
    seq_printf(m, "total_blinks      : %lu\n", b);
    seq_printf(m, "last_write_len    : %u bytes\n", lwl);
    seq_printf(m, "timer_enabled     : %s\n",  te ? "yes" : "no");
    seq_printf(m, "timer_interval_ms : %lu ms\n", ti);
    seq_printf(m, "blink_count       : %d\n", bc);
    seq_printf(m, "blink_delay_ms    : %d ms\n", bd);
    seq_printf(m, "work_pending      : %s\n",  wp ? "yes" : "no");
    return 0;
}

static int ddk_proc_open(struct inode *inode, struct file *file)
{
    /* Pass driver state as private data for seq_file */
    return single_open(file, ddk_proc_show, pde_data(inode));
}

/* ------------------------------------------------------------------ */
/* Proc write — accepts runtime config commands                        */
/*                                                                     */
/* Supported commands:                                                 */
/*   timer 1          enable timer                                     */
/*   timer 0          disable timer                                    */
/*   interval <ms>    set timer interval                               */
/*   blink <n>        set blink count                                  */
/*   delay <ms>       set blink delay                                  */
/*   reset            zero all counters                                */
/* ------------------------------------------------------------------ */
static ssize_t ddk_proc_write(struct file *file, const char __user *ubuf,
                               size_t count, loff_t *ppos)
{
    struct ddk_ext_state *ext = pde_data(file_inode(file));
    char buf[64];
    char cmd[32];
    long val;
    unsigned long flags;
    size_t len = min(count, sizeof(buf) - 1);

    if (copy_from_user(buf, ubuf, len))
        return -EFAULT;
    buf[len] = '\0';

    /* Strip trailing newline */
    if (len > 0 && buf[len - 1] == '\n')
        buf[len - 1] = '\0';

    if (sscanf(buf, "%31s %ld", cmd, &val) < 1)
        return -EINVAL;

    spin_lock_irqsave(&ext->lock, flags);

    if (strcmp(cmd, "timer") == 0) {
        ext->timer_enabled = (val != 0);
        if (!ext->timer_enabled)
            /* Best-effort cancel; del_timer is safe here */
            timer_delete(&ext->activity_timer);
        pr_info("ddk_ext: timer %s\n", ext->timer_enabled ? "enabled" : "disabled");

    } else if (strcmp(cmd, "interval") == 0) {
        if (val > 0)
            ext->timer_interval_ms = (unsigned long)val;
        pr_info("ddk_ext: interval set to %lu ms\n", ext->timer_interval_ms);

    } else if (strcmp(cmd, "blink") == 0) {
        if (val > 0)
            ext->blink_count = (int)val;
        pr_info("ddk_ext: blink_count set to %d\n", ext->blink_count);

    } else if (strcmp(cmd, "delay") == 0) {
        if (val > 0)
            ext->blink_delay_ms = (int)val;
        pr_info("ddk_ext: blink_delay_ms set to %d\n", ext->blink_delay_ms);

    } else if (strcmp(cmd, "reset") == 0) {
        ext->total_usb_success = 0;
        ext->total_usb_fail    = 0;
        ext->total_blinks      = 0;
        ext->last_write_len    = 0;
        pr_info("ddk_ext: counters reset\n");

    } else {
        spin_unlock_irqrestore(&ext->lock, flags);
        pr_warn("ddk_ext: unknown command: %s\n", cmd);
        return -EINVAL;
    }

    spin_unlock_irqrestore(&ext->lock, flags);
    return count;
}

/* Modern proc_ops (kernel >= 5.6) */
static const struct proc_ops ddk_proc_fops = {
    .proc_open    = ddk_proc_open,
    .proc_read    = seq_read,
    .proc_write   = ddk_proc_write,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

/* ------------------------------------------------------------------ */
/* Public API — called from USB write path                             */
/* ------------------------------------------------------------------ */

void ddk_ext_on_write_success(struct ddk_ext_state *ext, size_t len)
{
    unsigned long flags;
    bool te;
    unsigned long interval;

    spin_lock_irqsave(&ext->lock, flags);
    ext->total_usb_success++;
    ext->last_write_len = (unsigned int)len;
    te       = ext->timer_enabled;
    interval = msecs_to_jiffies(ext->timer_interval_ms);
    spin_unlock_irqrestore(&ext->lock, flags);

    if (te) {
        /* Schedule timer; mod_timer re-arms if already pending */
        mod_timer(&ext->activity_timer, jiffies + interval);
    } else {
        /* Timer off: queue work directly for immediate blink */
        spin_lock_irqsave(&ext->lock, flags);
        if (!ext->work_pending) {
            ext->work_pending = true;
            schedule_work(&ext->blink_work);
        }
        spin_unlock_irqrestore(&ext->lock, flags);
    }
}
EXPORT_SYMBOL(ddk_ext_on_write_success);

void ddk_ext_on_write_fail(struct ddk_ext_state *ext)
{
    unsigned long flags;
    spin_lock_irqsave(&ext->lock, flags);
    ext->total_usb_fail++;
    spin_unlock_irqrestore(&ext->lock, flags);
}
EXPORT_SYMBOL(ddk_ext_on_write_fail);

/* ------------------------------------------------------------------ */
/* Init / Cleanup                                                       */
/* ------------------------------------------------------------------ */

int ddk_ext_init(struct ddk_ext_state *ext)
{
    spin_lock_init(&ext->lock);

    /* Default config */
    ext->total_usb_success  = 0;
    ext->total_usb_fail     = 0;
    ext->total_blinks       = 0;
    ext->last_write_len     = 0;
    ext->timer_enabled      = false;
    ext->timer_interval_ms  = DDK_TIMER_DEF_MS;
    ext->blink_count        = DDK_BLINK_DEF_CNT;
    ext->blink_delay_ms     = DDK_BLINK_DEF_MS;
    ext->work_pending       = false;

    /* Timer setup — does NOT start the timer, only initialises it */
    timer_setup(&ext->activity_timer, activity_timer_cb, 0);

    /* Workqueue work item */
    INIT_WORK(&ext->blink_work, blink_worker);

    /* Proc entry — pass ext as private data */
    ext->proc_entry = proc_create_data(DDK_PROC_NAME, 0666,
                                       NULL, &ddk_proc_fops, ext);
    if (!ext->proc_entry) {
        pr_err("ddk_ext: failed to create /proc/%s\n", DDK_PROC_NAME);
        return -ENOMEM;
    }

    pr_info("ddk_ext: initialized — /proc/%s created\n", DDK_PROC_NAME);
    return 0;
}
EXPORT_SYMBOL(ddk_ext_init);

void ddk_ext_cleanup(struct ddk_ext_state *ext)
{
    /* Remove proc entry first so no new writes arrive */
    if (ext->proc_entry) {
        remove_proc_entry(DDK_PROC_NAME, NULL);
        ext->proc_entry = NULL;
    }

    /* Stop timer — del_timer_sync waits for any running callback */
    timer_delete_sync(&ext->activity_timer);

    /* Cancel/flush pending work — cancel_work_sync waits for worker */
    cancel_work_sync(&ext->blink_work);

    pr_info("ddk_ext: cleanup done\n");
}
EXPORT_SYMBOL(ddk_ext_cleanup);
