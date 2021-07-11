#include <linux/module.h>	// for init_module() 
#include <linux/fs.h>		// for register_chrdev()
#include <linux/sched.h>	// for interruptible_sleep_on() 
#include <asm/uaccess.h>	// for get_user(), put_user()

#define RINGSIZE 512

static char modname[] = "wait1";
static int my_major = 40;

// driver's data-structures
static wait_queue_head_t  wq;
static volatile int head, tail;
static unsigned char ring[ RINGSIZE ];

int condition=0;


static ssize_t
my_write( struct file *file, const char *buf, size_t count, loff_t *pos )
{
	int	prev;

	// sleep if necessary until the ringbuffer has room for new data 
	prev = ( head - 1 ) % RINGSIZE;
	while ( prev == tail )	// the ringbuffer is aready full 
		{
		condition=0;
		wait_event_interruptible ( wq,condition!=0);
		if ( signal_pending( current ) ) return -ERESTARTSYS;
		}

	// insert a new byte of data into our ringbuffer
	if ( get_user( ring[ tail ], buf ) ) return -EFAULT; 
	tail = ( 1 + tail ) % RINGSIZE;

	condition=1;
	// and awaken any sleeping readers 
	wake_up_interruptible( &wq );
	return	1;
}


static ssize_t
my_read( struct file *file, char *buf, size_t count, loff_t *pos )
{
	int	next;

	// sleep if necessary until the ringbuffer has some data in it
	next = head;
	while ( next == tail ) 	// the ringbuffer is already empty
		{
		condition=0;
		wait_event_interruptible ( wq ,condition!=0);
		if ( signal_pending( current ) ) return -ERESTARTSYS;
		}

	// remove a byte of data from our ringbuffer
	if ( put_user( ring[ head ], buf ) ) return -EFAULT;
	head = ( 1 + head ) % RINGSIZE;

	condition=1;

	// and awaken any sleeping writers
	wake_up_interruptible( &wq );
	return	1;
}


static struct file_operations 
my_fops =	{
		owner:		THIS_MODULE,
		write:		my_write,
		read:		my_read,
		};


int init_module( void )
{
	printk( "<1>\nInstalling \'%s\' module ", modname );
	printk( "(major=%d) \n", my_major );

	// initialize our wait-queue structure
	init_waitqueue_head( &wq );
	
	// register this device-driver with the kernel
	return	register_chrdev( my_major, modname, &my_fops );
}


void cleanup_module( void )
{
	printk( "<1>Removing \'%s\' module\n", modname );

	// unregister this device-driver 
	unregister_chrdev( my_major, modname );
}


MODULE_LICENSE("GPL");
