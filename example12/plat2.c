#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

static int Major;

int plat2_open(struct inode *inode, struct file *file)
{
	printk("Open from userspace\n");
	return 0;
}

int plat2_release(struct inode *inode, struct file *file)
{
	printk("Release from userspace\n");
	return 0;
}

ssize_t plat2_write(struct file *file, const char __user * buf, size_t count, loff_t *ppos)
{
	printk("write from userspace\n");
	return 0;
}

ssize_t plat2_read( struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	printk("Read from userspace\n");
	return 0;
}


static const struct file_operations plat2_device_fops = {
	.read = plat2_read,
	.write = plat2_write,
	.open = plat2_open,
	.release = plat2_release,
};

static int plat2_probe_callback_fn(struct platform_device *pdev)
{
	printk("Binding/Initializing probe callback\n");

	Major = register_chrdev(0, "plat2", &plat2_device_fops);

	if (Major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}

	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", "plat2", Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

	return 0;
}


static int plat2_remove_callback_fn(struct platform_device *pdev)
{
	/*
	* Unregister the device
	*/
	unregister_chrdev(Major, "plat2");

	return 0;
}

static struct platform_driver plat2_driver = {
	.probe = plat2_probe_callback_fn,
	.remove = plat2_remove_callback_fn,
	.driver = {
		.name = "plat2",
		.owner = THIS_MODULE,
	},
};

static int __init init_callback_fn(void)
{
	printk("Initializing Plat2\n");
	return platform_driver_register(&plat2_driver);
}

static void __exit exit_callback_fn(void)
{
	printk("Exiting Plat2\n");
	platform_driver_unregister(&plat2_driver);
}


MODULE_AUTHOR("test");
MODULE_DESCRIPTION("plat2");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

module_init(init_callback_fn);
module_exit(exit_callback_fn);
