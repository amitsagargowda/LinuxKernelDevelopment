#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME     "testchar"
static int num_major;

static int test_open(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "test_open called\n");
        return 0;
}

static int test_release(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "test_release called\n");
        return 0;
}

static ssize_t test_read(struct file *filp,  
                           char *buf,  
                           size_t len,  
                           loff_t * off)
{
        printk(KERN_INFO "test_read called\n");     
        return 0;
}

static ssize_t test_write(struct file *filp,
                            const char *buf,
                            size_t len,
                            loff_t * off)
{

        printk(KERN_INFO "test_write called\n"); 
        return 0;
}



static struct file_operations fops= {
        .open = test_open,
        .release = test_release,
        .read = test_read,
        .write = test_write,
};


static int __init test_init(void)
{

        printk(KERN_INFO "Entering Test Character Driver \n");

        num_major=register_chrdev(0, DEVICE_NAME, &fops);
        printk(KERN_INFO "Major Number = %d \n",num_major);
        printk(KERN_INFO "Name =  %s \n",DEVICE_NAME);
        printk(KERN_INFO "Generate the device file with\
                mknod /dev/%s c %d 0 \n",DEVICE_NAME,num_major);

        return 0 ;

}

static void __exit test_cleanup(void)
{
        unregister_chrdev(num_major, DEVICE_NAME);
        printk(KERN_INFO "Exiting Test Character Driver \n");
}

module_init(test_init);
module_exit(test_cleanup);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("test");                       
MODULE_DESCRIPTION("Character Device Driver");            
MODULE_SUPPORTED_DEVICE("testchar");