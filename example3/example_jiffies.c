/*  
 *  example_jiffies.c
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/jiffies.h>

int init_module(void)
{
	printk(KERN_INFO "Entering Jiffies Example\n");
	printk(KERN_INFO "The jiffies value=%ld\n",jiffies);

	return 0;
}

void cleanup_module(void)
{
	printk(KERN_INFO "Exiting Jiffies Example\n");
}