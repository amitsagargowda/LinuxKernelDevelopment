#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/mm.h>

MODULE_DESCRIPTION("RD WR");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.1.1");

#define MAXBYTES 10

static char *kernel_buffer;

static int openFunc(struct inode *openNode, struct file *openFile)
{
    char *openBuff = kzalloc(PATH_MAX, GFP_KERNEL);
    printk(KERN_INFO "Opening device file %s \n", file_path(openFile, openBuff, PATH_MAX));
    kfree(openBuff);
   
    return nonseekable_open(openNode, openFile);
}

static ssize_t readFunc(struct file *readFile, char __user *readBuffer, size_t readCount, loff_t *readOffset)
{
    printk(KERN_INFO "Inside reader \n");
   
    printk(KERN_INFO "Request to read %ld bytes from driver to transmit to user space \n", readCount);
   
    if (copy_to_user(readBuffer, kernel_buffer, readCount))
    {
        printk(KERN_INFO "Issue with copying data to user buffer \n");
    }
    *readOffset += readCount;
    return readCount;
}

static ssize_t writeFunc(struct file *writeFIle, const char __user *writeBuffer, size_t writeCount, loff_t *writeOffset)
{
    printk(KERN_INFO "Inside writer = %ld \n", writeCount);
       
    copy_from_user(kernel_buffer, writeBuffer, writeCount);
    kernel_buffer[writeCount] = '\0';
    return writeCount;
}

static int releaseFunc(struct inode *releaseNode, struct file *releaseFile)
{
    char *file_buff = kzalloc(PATH_MAX, GFP_KERNEL);
   
    printk(KERN_INFO "Closing the driver file %s \n", file_path(releaseFile, file_buff, PATH_MAX));
   
    return 0;
}

static const struct file_operations misc_driver_fops =
{
    .open    = openFunc,
    .read    = readFunc,
    .write   = writeFunc,
    .release = releaseFunc,
    .llseek  = no_llseek,
};

static struct miscdevice misc_driver =
{
    .name  = "test",
    .mode  = 0666,
    .minor = MISC_DYNAMIC_MINOR,
    .fops  = &misc_driver_fops,
};

static int __init startUp(void)
{
    int retcode;
   int m;
    printk(KERN_INFO "Registering module \n");
    retcode = misc_register(&misc_driver);
    if (retcode)
    {
        printk(KERN_ERR "Error in creating misc driver \n");
        return retcode;
    }
    kernel_buffer = kvmalloc(100, GFP_KERNEL);
    m = strlcpy(kernel_buffer, "initmsg", 8);
    printk(KERN_INFO "Misc device registered with minor = %d ; m = %d , dev node = %s \n", misc_driver.minor,m, misc_driver.name);
    return 0;
}

static void __exit terminate(void)
{
    printk(KERN_INFO "Removing module \n");
    misc_deregister(&misc_driver);
}

module_init(startUp);
module_exit(terminate);

