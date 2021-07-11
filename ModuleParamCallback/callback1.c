#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>   /* Needed to use module_param */

int  intVal          = 0;            /* Integer param */ 
char *charPVal       = "HELLO";      /* Char Pointer */
int  intArray[4]     = {-1, -2} ;    /* Integer array passing */
int  intValChgNotify = 9;            /* To show how we can get notification */
int  count_of_array_elements ;
int  i = 0;

MODULE_AUTHOR("Abha Patidar");
MODULE_DESCRIPTION("Parameter Passing");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.1.1");

MODULE_PARM_DESC(intVal,          "Integer Variable");
MODULE_PARM_DESC(charPVal,        "Character Pointer Variable");
MODULE_PARM_DESC(intArray,        "Integer Array Variable");
MODULE_PARM_DESC(intValChgNotify, "Integer Variable to see call back getter/setter functions");

int setNewValue (const char *newValue, const struct kernel_param *kp)
{
    int res = param_set_int(newValue, kp);
    if (res == 0){
        printk(KERN_INFO "Setter method called \n");
        printk(KERN_INFO "New value is = %d \n", intValChgNotify);
        return 0;
    }
    
    return -1;
}

int getCurrentValue (char *valueBuffer, const struct kernel_param *kp)
{
    int res;
    printk(KERN_INFO "Getter method called \n");
    
    res = param_get_int(valueBuffer, kp);
    if (res >= 0){
        printk(KERN_INFO "Getter method called \n");
        printk(KERN_INFO "Current value is = %d \n", intValChgNotify);
        return res;
    }
    
    printk(KERN_INFO "Error getting value  \n" );
    return -1;
}

static struct kernel_param_ops kParamOps = 
{
    .set = setNewValue     ,   // Use our setter. This is called when value is set from outside 
    .get = getCurrentValue ,
    //.get = param_get_int,      // getCurrentValue ,   // This is standard getter = param_get_int
};


module_param(intVal,             int   ,                               S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
module_param(charPVal,           charp ,                               S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
module_param_array(intArray,     int,        &count_of_array_elements, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
module_param_cb(intValChgNotify, &kParamOps, &intValChgNotify,         S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);

static int __init startUpModule(void)
{
    printk(KERN_INFO "==  Trying to insert module into kernel memory ... \n");
    
    printk(KERN_INFO "Value of Integer intVal is %d \n", intVal);
    printk(KERN_INFO "Value of Character Pointer charPval is %s \n", charPVal);
    printk(KERN_INFO "Value of Integer to be notified param is %d \n", intValChgNotify);
   
    for (i = 0 ; i < sizeof(intArray)/sizeof(int) ; i++)
    {
        printk(KERN_INFO "Value of element number %d = %d \n", i, intArray[i]);
    }
    printk(KERN_INFO "Number of array elements got %d \n", count_of_array_elements);
    
    printk(KERN_INFO "==  Module inserted into kernel memory\n");
    
    return 0;
}

static void __exit exitModule(void)
{
    printk(KERN_INFO "Kernel Module removed successfully\n");
}

module_init(startUpModule);
module_exit(exitModule);

