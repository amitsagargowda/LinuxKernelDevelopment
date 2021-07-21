#include <linux/module.h>
#include <linux/kernel.h>           //used for do_exit()
#include <linux/threads.h>          //used for allow_signal
#include <linux/kthread.h>          //used for kthread_create
#include <linux/delay.h>            //used for ssleep()
#include <linux/sched.h>
#include <linux/sched/signal.h>

static struct task_struct *worker_task,*default_task;
#define WORKER_THREAD_DELAY 4
#define DEFAULT_THREAD_DELAY 6

MODULE_LICENSE("GPL");
MODULE_AUTHOR("test");
MODULE_DESCRIPTION("Kernel thread example");
MODULE_ALIAS("Threading");


static int worker_task_handler_fn(void *arguments)
{
	allow_signal(SIGKILL);

	while(!kthread_should_stop()){
		printk(KERN_ERR "Worker thread executing on system CPU:%d \n",get_cpu());
		ssleep(WORKER_THREAD_DELAY);

		if (signal_pending(worker_task))
			            break;
	}

	printk(KERN_ERR "Worker task exiting\n");

	worker_task=NULL;
	
	do_exit(0);

	return 0;
}

static int default_task_handler_fn(void *arguments)
{
	allow_signal(SIGKILL);

	while(!kthread_should_stop())
	{
		printk(KERN_ERR "Default thread executing on system CPU:%d \n",get_cpu());
		ssleep(DEFAULT_THREAD_DELAY);

	    if (signal_pending(default_task))
		            break;
	}

	default_task=NULL;

	printk(KERN_ERR "Default task exiting\n");

	do_exit(0);

	return 0;
}

static int __init kernel_thread_init(void)
{

	printk(KERN_ERR "Initializing kernel mode thread example module\n");

	worker_task = kthread_create(worker_task_handler_fn,
			(void*)"worker_thread","worker_thread");

	if(worker_task)
		printk(KERN_ERR "Worker task created successfully\n");
	else
		printk(KERN_ERR "Worker task error while creating\n");

	default_task = kthread_create(default_task_handler_fn,
				(void*)"default thread","default_thread");

	wake_up_process(worker_task);
	wake_up_process(default_task);

	if(worker_task)
		printk(KERN_ERR "Worker thread running\n");
	else
		printk(KERN_ERR "Worker task can't start\n");

	if(default_task)
		printk(KERN_ERR "Default thread running\n");
	else
		printk(KERN_ERR "Default task can't start\n");

	return 0;
}

static void __exit kernel_thread_exit(void)
{	

	printk(KERN_ERR "Module removing from kernel, threads stopping\n");

	if(worker_task)
		kthread_stop(worker_task);

	printk(KERN_ERR "Worker task stopped\n");

	if(default_task)
		kthread_stop(default_task);

	printk(KERN_ERR "Default task stopped\n");

}

module_init(kernel_thread_init);
module_exit(kernel_thread_exit);

