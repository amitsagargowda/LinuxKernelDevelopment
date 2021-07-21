#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h> // for spinlock

MODULE_LICENSE("GPL");
MODULE_AUTHOR("test");
MODULE_DESCRIPTION("Working code for Kernel Threads");

static struct task_struct *thread1;
static struct task_struct *thread2;

static int thread2_function(void *unused)
{
        while(!kthread_should_stop())
        {
                printk(KERN_ALERT "Thread 2 running...\n");
                ssleep(5);
        }
        printk(KERN_ALERT "Stopping thread 2...\n");
        do_exit(0);
        return 0;
}


static int thread1_function(void *unused)
{
    while(!kthread_should_stop())
    {
        printk(KERN_ALERT "Thread 1 running...\n");
        ssleep(5);
    }
    printk(KERN_ALERT "Stopping thread 1 ...\n");
    do_exit(0);
    return 0;
}

static int __init init_thread(void)
{
    printk(KERN_INFO "Thread creating ...\n");
    thread1 = kthread_create(thread1_function,NULL,"mythread1");
    thread2 = kthread_create(thread2_function,NULL,"mythread2");
    if(thread1 &&  thread2)
    {
        printk(KERN_INFO "Thread Created Sucessfully\n");
        wake_up_process(thread1);
        wake_up_process(thread2);
    }
    else
    {
        printk(KERN_ALERT "Thread Creation Failed\n");
    }
    return 0;
}

static void __exit cleanup_thread(void)
{
    printk(KERN_INFO "Cleaning up ...\n");
    kthread_stop(thread1);
    kthread_stop(thread2);
    printk(KERN_ALERT "Thread stopped");
}
module_init(init_thread)
module_exit(cleanup_thread)
