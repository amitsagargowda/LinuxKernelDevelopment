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
static spinlock_t my_lock = __SPIN_LOCK_UNLOCKED(); //SPIN_LOCK_UNLOCKED;

//spinlock_t my_lock;
//spin_lock_init(&my_lock);
int lock_var;

static struct task_struct *thread1;
static struct task_struct *thread2;
//static int reader();

int reader(void)
{
    int z;
    spin_lock(&my_lock);
    if (spin_is_locked(&my_lock))
                {
                        printk(KERN_INFO "Read Locked\n");
                }
                else
                {
                        printk(KERN_INFO "Read UnLocked\n");
                }

     z = lock_var;
    spin_unlock(&my_lock);
    if (spin_is_locked(&my_lock))
                {
                        printk(KERN_INFO "Read Locked\n");
                }
                else
                {
                        printk(KERN_INFO "Read UnLocked\n");
                }

    return z;
}


static int thread2_function(void *unused)
{

        int mul=2,i=2;
        spin_lock(&my_lock);
        lock_var = 1;
        if (spin_is_locked(&my_lock))
        {
                printk(KERN_INFO "Locked\n");
        }
        spin_unlock(&my_lock);
        if (!spin_is_locked(&my_lock))
        {
                printk(KERN_INFO "UnLocked\n");
        }

        while(reader())
        {
                mul = mul* i;
                i++;
                printk(KERN_ALERT "Thread 2 running...\n");
                printk(KERN_NOTICE "mul  is %d ",mul);
                ssleep(5);
        }
        printk(KERN_ALERT "Stopping thread 2...\n");
        do_exit(0);
        return 0;
}


static int thread1_function(void *unused)
{

    int sum=0,i=1;
    spin_lock(&my_lock);
    lock_var = 1;
    if (spin_is_locked(&my_lock))
    {
        printk(KERN_INFO "Locked\n");
    }
    spin_unlock(&my_lock);
    if (!spin_is_locked(&my_lock))
        {
                printk(KERN_INFO "UnLocked\n");
        }

    while(reader())
    {
        sum = sum + i;
        i++;
        printk(KERN_ALERT "Thread 1 running...\n");
        printk(KERN_NOTICE "sum  is %d ",sum);
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
    int ret,ret1;
    spin_lock(&my_lock);
    if (spin_is_locked(&my_lock))
    {
         printk(KERN_INFO "Locked\n");
    }

    lock_var = 0;
    spin_unlock(&my_lock);
    if (!spin_is_locked(&my_lock))
    {
        printk(KERN_INFO "UnLocked\n");
    }


    ret = kthread_stop(thread1);
    ret1 = kthread_stop(thread2);
    if(!ret && !ret1)
        printk(KERN_ALERT "Thread stopped");
}
module_init(init_thread)
module_exit(cleanup_thread)
