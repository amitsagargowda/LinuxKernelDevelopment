#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

/* Basic devic operation */
static int chrdevic_open(struct inode *inode, struct file *filep);
static int chrdevic_release(struct inode *inode, struct file *filep);
static ssize_t chrdevic_read(struct file *filep, char *buf, size_t lbuf, loff_t *ppos);
static ssize_t chrdevic_write(struct file *filep, const char *buf, size_t lbuf, loff_t *ppos);
static loff_t chrdevic_lseek(struct file *filep, loff_t offset, int orig);
static long chrdevic_ioctl(struct file *filep, unsigned int cmd, unsigned long args);

/* file operations structure */
static struct file_operations chrdevic_fileops;

 
/* Variable declaration */
#define CHAR_DEVIC_NAME "SAMPLE_DEVIC"
#define MAX_LENGTH 4000

static int chrdevic_id;
static struct cdev *chrdevic;
static dev_t devicnums;

/**
 * Driver initialization routine.It shows how to register a char device
 * Look it closely, important routine to understand a char driver.
  
 * */
__init int init_module(void)
{
	int  ret;

	/* Initialize the file operations structure object as we did earlier */
	chrdevic_fileops.owner = THIS_MODULE;
	chrdevic_fileops.read = chrdevic_read;
	chrdevic_fileops.write = chrdevic_write;
	chrdevic_fileops.open = chrdevic_open;
	chrdevic_fileops.release = chrdevic_release;
	chrdevic_fileops.llseek = chrdevic_lseek;
	chrdevic_fileops.unlocked_ioctl = chrdevic_ioctl;

	/* Acquire the major and minor numbers for your driver module */
	/* We are passing 0 in the second argument and passing 1 in the */
	/* third argument. That means we want to request only one minor number for */  
	/* this major number and so that minor number would be 0 */

	ret = alloc_chrdev_region(&devicnums, 0 ,1 , "SAMPLE_DEVIC");

	chrdevic_id = MAJOR(devicnums);//Get the major no

	chrdevic = cdev_alloc();//Get an allocated cdev structure

    	/*Register the character Device*/
	chrdevic->owner = THIS_MODULE;
	chrdevic->ops = &chrdevic_fileops;

    	/* Add this to the kernel */
	ret = cdev_add(chrdevic, devicnums, 1);
	if(ret < 0)
	{
		printk("Error registering devic driver with MAJOR NO [%d]\n",chrdevic_id);
		return ret;
	}
	printk("Devic successfully with MAJOR NUMBER [%d]\n",chrdevic_id);
	printk("Devic successfully with MINOR NUMBER [%d]\n",MINOR(devicnums));
	return 0;
}


/* Module Cleanup. Simply returns back the major,minor number and the cdev   object resources back.*/
__exit void cleanup_module(void)
{
	printk("\n MODULE Removed");
	unregister_chrdev_region(devicnums,1);
	cdev_del(chrdevic);
}

static int chrdevic_open(struct inode *inode, struct file *filep)
{
	static int counter = 0;
	counter++;
	printk("Number of times open() was called: %d\n",counter);
	printk("Process id of current process: %d\n",current->pid);
	return 0;
}

static int chrdevic_release(struct inode *inode, struct file *filep)
{
	printk("close called\n");
	return 0;
}

static ssize_t chrdevic_read(struct file *filep, char *buf, size_t lbuf, loff_t *ppos)
{
	printk("READ called\n");
	return 0;
}

static ssize_t chrdevic_write(struct file *filep, const char *buf, size_t lbuf, loff_t *ppos)
{
	printk("Write called\n");
	return 0;
}

static loff_t chrdevic_lseek(struct file *filep, loff_t offset, int orig)
{
	printk("lseek called\n");
	return 0;
}

static long chrdevic_ioctl(struct file *filep, unsigned int cmd, unsigned long args)
{
	printk("ioctl called cmd=%d args=%ld\n",cmd,args);
	return 0;
}

MODULE_AUTHOR("test");
MODULE_DESCRIPTION("A Basic Character Devic Driver");
