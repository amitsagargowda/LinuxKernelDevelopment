#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");

static void my_tasklet_handler(unsigned long flag);
DECLARE_TASKLET(my_tasklet, my_tasklet_handler, 0);

static void my_tasklet_handler(unsigned long flag)
{
	tasklet_disable(&my_tasklet);
	printk("my_tasklet run: do what the tasklet want to do....\n");
	tasklet_enable(&my_tasklet);
}

static int hello_tasklet_init(void)
{
	printk("module init start. \n");
	printk("Hello tasklet!\n");
	tasklet_schedule(&my_tasklet);
	printk("module init end.\n");
	return 0;
}

static void hello_tasklet_exit(void)
{
	tasklet_kill(&my_tasklet);
	printk("Goodbye, tasklet!\n");
}

module_init(hello_tasklet_init);
module_exit(hello_tasklet_exit);
