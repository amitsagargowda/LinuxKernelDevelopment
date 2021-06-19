/* file2.c - The simplest kernel module. */
#include <linux/module.h> /* Needed by all modules */ 
#include <linux/kernel.h> /* Needed for KERN_ALERT */ 
#include <linux/init.h>   /* Needed for macros */

void example1_stop(void) { 
  printk(KERN_ALERT "Exiting example1\n"); 
} 

module_exit (example1_stop);