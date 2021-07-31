#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>
#include<linux/proc_fs.h>

#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)
 
int32_t value = 0;
char test_array[20]="try_proc_array\n";
static int len = 1;
 
 
dev_t dev = 0;
static struct class *dev_class;
static struct cdev test_cdev;
static struct proc_dir_entry *parent;
static int      __init test_driver_init(void);
static void     __exit test_driver_exit(void);
/*************** Driver Functions **********************/
static int      test_open(struct inode *inode, struct file *file);
static int      test_release(struct inode *inode, struct file *file);
static ssize_t  test_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  test_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     test_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
 
/***************** Procfs Functions *******************/
static int      open_proc(struct inode *inode, struct file *file);
static int      release_proc(struct inode *inode, struct file *file);
static ssize_t  read_proc(struct file *filp, char __user *buffer, size_t length,loff_t * offset);
static ssize_t  write_proc(struct file *filp, const char *buff, size_t len, loff_t * off);
/*
** File operation sturcture
*/
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = test_read,
        .write          = test_write,
        .open           = test_open,
        .unlocked_ioctl = test_ioctl,
        .release        = test_release,
};
/*
** procfs operation sturcture
*/
static struct proc_ops proc_fops = {
        .proc_open = open_proc,
        .proc_read = read_proc,
        .proc_write = write_proc,
        .proc_release = release_proc
};

/*
** This function will be called when we open the procfs file
*/
static int open_proc(struct inode *inode, struct file *file)
{
    pr_info("proc file opend.....\t");
    return 0;
}
/*
** This function will be called when we close the procfs file
*/
static int release_proc(struct inode *inode, struct file *file)
{
    pr_info("proc file released.....\n");
    return 0;
}
/*
** This function will be called when we read the procfs file
*/
static ssize_t read_proc(struct file *filp, char __user *buffer, size_t length,loff_t * offset)
{
    pr_info("proc file read.....\n");
    if(len)
    {
        len=0;
    }
    else
    {
        len=1;
        return 0;
    }
    
    if( copy_to_user(buffer,test_array,20) )
    {
        pr_err("Data Send : Err!\n");
    }
 
    return length;;
}
/*
** This function will be called when we write the procfs file
*/
static ssize_t write_proc(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    pr_info("proc file wrote.....\n");
    
    if( copy_from_user(test_array,buff,len) )
    {
        pr_err("Data Write : Err!\n");
    }
    
    return len;
}
/*
** This function will be called when we open the Device file
*/
static int test_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}
/*
** This function will be called when we close the Device file
*/
static int test_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}
/*
** This function will be called when we read the Device file
*/
static ssize_t test_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read function\n");
        return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t test_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write Function\n");
        return len;
}
/*
** This function will be called when we write IOCTL on the Device file
*/
static long test_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
         switch(cmd) {
                case WR_VALUE:
                        if( copy_from_user(&value ,(int32_t*) arg, sizeof(value)) )
                        {
                                pr_err("Data Write : Err!\n");
                        }
                        pr_info("Value = %d\n", value);
                        break;
                case RD_VALUE:
                        if( copy_to_user((int32_t*) arg, &value, sizeof(value)) )
                        {
                                pr_err("Data Read : Err!\n");
                        }
                        break;
                default:
                        pr_info("Default\n");
                        break;
        }
        return 0;
}
 
/*
** Module Init function
*/
static int __init test_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "test_Dev")) <0){
                pr_info("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        /*Creating cdev structure*/
        cdev_init(&test_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&test_cdev,dev,1)) < 0){
            pr_info("Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if((dev_class = class_create(THIS_MODULE,"test_class")) == NULL){
            pr_info("Cannot create the struct class\n");
            goto r_class;
        }
 
        /*Creating device*/
        if((device_create(dev_class,NULL,dev,NULL,"test_device")) == NULL){
            pr_info("Cannot create the Device 1\n");
            goto r_device;
        }
        
        /*Create proc directory. It will create a directory under "/proc" */
        parent = proc_mkdir("test",NULL);
        
        if( parent == NULL )
        {
            pr_info("Error creating proc entry");
            goto r_device;
        }
        
        /*Creating Proc entry under "/proc/test/" */
        proc_create("test_proc", 0666, parent, &proc_fops);
 
        pr_info("Device Driver Insert...Done!!!\n");
        return 0;
 
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}
 
/*
** Module exit function
*/
static void __exit test_driver_exit(void)
{
        /* remove complete /proc/test */
        proc_remove(parent);
        
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&test_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
}
 
module_init(test_driver_init);
module_exit(test_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Test");
MODULE_DESCRIPTION("Simple Linux device driver (procfs)");
MODULE_VERSION("1.0");
