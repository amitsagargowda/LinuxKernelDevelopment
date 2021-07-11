#include <linux/module.h>  
#include <linux/init.h>  
#include <linux/timer.h> // for timer_list API
#include <linux/param.h> // for HZ 
#include <linux/jiffies.h> // for jiffies 
#include <linux/interrupt.h>
  
void tasklet_demo_handle(unsigned long data) {
	printk("%s\n", __func__);
	printk("%ld\n", data);
}
DECLARE_TASKLET(tasklet_demo,  tasklet_demo_handle,  0);

static void ISR_handler(struct timer_list * data) {
        tasklet_schedule(&tasklet_demo);
        return;
}

DEFINE_TIMER(timer, ISR_handler);

static int __init tasklet_demo_init(void)  
{  
	printk("init tasklet demo.\n");
	timer.expires = jiffies + HZ*10;
	add_timer(&timer);
   	return 0;    
}  
  
static void __exit tasklet_demo_exit(void)  
{  
	printk("exit tasklet demo.\n");
	del_timer_sync(&timer);
}  
MODULE_LICENSE("GPL");
MODULE_AUTHOR("test");  
module_init(tasklet_demo_init);  
module_exit(tasklet_demo_exit);
