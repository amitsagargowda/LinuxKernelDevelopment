/* example1.c - The simplest kernel module. */
#include <linux/module.h> /* Needed by all modules */ 
#include <linux/kernel.h> /* Needed for KERN_ALERT */ 
#include <linux/init.h>   /* Needed for macros */

int example1_init(void) { 
  printk(KERN_ALERT "Initing example1\n"); /* A non 0 return means init_module failed; module can't be loaded. */
  return 0; 
} 

void example1_stop(void) { 
  printk(KERN_ALERT "Exiting example1\n"); 
} 

module_init (example1_init);
module_exit (example1_stop);

