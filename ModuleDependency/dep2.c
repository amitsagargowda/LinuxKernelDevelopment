/* dep2.c - The simplest kernel module. */
#include <linux/module.h> /* Needed by all modules */ 
#include <linux/kernel.h> /* Needed for KERN_ALERT */ 
#include <linux/init.h>   /* Needed for macros */

extern void dep1_func(void);

int dep2_init(void) { 
  printk(KERN_INFO "Initing dep2\n"); /* A non 0 return means init_module failed; module can't be loaded. */
  dep1_func();
  return 0; 
} 

void dep2_stop(void) { 
  printk(KERN_INFO "Exiting dep2\n"); 
} 

module_init (dep2_init);
module_exit (dep2_stop);

