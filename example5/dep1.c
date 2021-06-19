/* dep1.c - The simplest kernel module. */
#include <linux/module.h> /* Needed by all modules */ 
#include <linux/kernel.h> /* Needed for KERN_ALERT */ 
#include <linux/init.h>   /* Needed for macros */

void dep1_func(void){
	printk(KERN_INFO "I am in dep1 func\n");
}

int dep1_init(void) { 
  printk(KERN_INFO "Initing dep1\n"); /* A non 0 return means init_module failed; module can't be loaded. */
  return 0; 
} 

void dep1_stop(void) { 
  printk(KERN_INFO "Exiting dep1\n"); 
} 

module_init (dep1_init);
module_exit (dep1_stop);
EXPORT_SYMBOL(dep1_func);

