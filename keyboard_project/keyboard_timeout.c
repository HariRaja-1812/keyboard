#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "kb_timeout"
#define KEYBOARD_IRQ 1
#define TIMEOUT_SEC 5

static int major;
static struct timer_list my_timer;
static unsigned long last_activity;
static int timeout_occurred = 0;

/* Keyboard IRQ Handler */
static irqreturn_t keyboard_irq_handler(int irq, void *dev_id)
{
    last_activity = jiffies;

    printk(KERN_INFO "Keyboard Activity Detected\n");

    return IRQ_HANDLED;
}

/* Timer Callback Function */
static void timer_callback(struct timer_list *t)
{
    unsigned long elapsed;

    elapsed = jiffies - last_activity;

    if (elapsed > nsecs_to_jiffies(TIMEOUT_SEC))
    {
        printk(KERN_INFO "Keyboard Timeout Detected\n");

        timeout_occurred = 1;

        /* Reset activity time */
        last_activity = jiffies;
    }

    /* Restart timer after 1 second */
    mod_timer(&my_timer, jiffies + HZ);
}

/* Read Function */
static ssize_t driver_read(struct file *file,char __user *buf,size_t len,loff_t *off)
{
    char message[100];
    int bytes;

    /* EOF condition */
    if (*off > 0)
        return 0;

    if (timeout_occurred)
    {
        bytes = sprintf(message,"Keyboard timeout detected\n");

        timeout_occurred = 0;
    }
    else
    {
        bytes = sprintf(message,"No timeout\n");
    }

    if (copy_to_user(buf, message, bytes))
        return -EFAULT;

    *off = bytes;

    return bytes;
}

/* File Operations */
static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .read = driver_read,
};

/* Module Initialization */
static int __init keyboard_timeout_init(void)
{
    int result;

    printk(KERN_INFO "Keyboard Timeout Driver Loaded\n");

    last_activity = jiffies;

    /* Register Character Device */
    major = register_chrdev(0,DEVICE_NAME,&fops);

    if (major < 0)
    {
        printk(KERN_ERR "Character Device Registration Failed\n");
        return major;
    }

    printk(KERN_INFO "Major Number = %d\n", major);

    /* Register Keyboard IRQ */
    result = request_irq(KEYBOARD_IRQ,keyboard_irq_handler,IRQF_SHARED,"keyboard_timeout",(void *)&keyboard_irq_handler);

    if (result)
    {
        printk(KERN_ERR "IRQ Registration Failed\n");

        unregister_chrdev(major, DEVICE_NAME);

        return result;
    }

    /* Setup Timer */
    timer_setup(&my_timer, timer_callback, 0);

    mod_timer(&my_timer, jiffies + HZ);

    printk(KERN_INFO "Timer Started\n");

    return 0;
}

/* Module Exit */
static void __exit keyboard_timeout_exit(void)
{
    /* Delete Timer */
    del_timer(&my_timer);

    /* Free IRQ */
    free_irq(KEYBOARD_IRQ,(void *)&keyboard_irq_handler);

    /* Unregister Character Device */
    unregister_chrdev(major,DEVICE_NAME);

    printk(KERN_INFO "Keyboard Timeout Driver Removed\n");
}

module_init(keyboard_timeout_init);
module_exit(keyboard_timeout_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Keyboard Timeout Detection Driver");
MODULE_VERSION("1.0");
