#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

	static int Major;

int plat_open(struct inode *inode, struct file *file)
{
	printk("Open from userspace\n");
	return 0;
}

int plat_release(struct inode *inode, struct file *file)
{
	printk("Release from userspace\n");
	return 0;
}

ssize_t plat_write(struct file *file, const char __user * buf, size_t count, loff_t *ppos)
{
	printk("Write from userspace\n");
	return 0;
}

ssize_t plat_read( struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	printk("Read from userspace\n");
	return 0;
}


static const struct file_operations plat_device_fops = {
	.read = plat_read,
	.write = plat_write,
	.open = plat_open,
	.release = plat_release,
};



static int plat_probe_callback_fn(struct platform_device *pdev)
{
	printk("Binding/Initializing probe callback\n");

  Major = register_chrdev(0, "plat1", &plat_device_fops);

  if (Major < 0) {
    printk(KERN_ALERT "Registering char device failed with %d\n", Major);
    return Major;
  }

  printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
  printk(KERN_INFO "the driver, create a dev file with\n");
  printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", "plat1", Major);
  printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
  printk(KERN_INFO "the device file.\n");
  printk(KERN_INFO "Remove the device file and module when done.\n");

	return 0;
}


static int plat_remove_callback_fn(struct platform_device *pdev)
{
  /*
   * Unregister the device
   */
  unregister_chrdev(Major, "plat1");

	return 0;
}


static struct platform_driver plat1_driver = {
	.probe = plat_probe_callback_fn,
	.remove = plat_remove_callback_fn,
	.driver = {
		.name = "plat1",
		.owner = THIS_MODULE,
	},
};
	struct platform_device *pdev;
	
static int __init init_callback_fn(void)
{
	struct platform_device *pdev;
	int err;
	printk("Initializing plat1 driver\n");
	
	pdev = platform_device_register_simple(KBUILD_MODNAME, -1, NULL, 0);
	if (IS_ERR(pdev)) {
		pr_err("Device allocation failed\n");
		return PTR_ERR(pdev);
	}

	err = platform_driver_probe(&plat1_driver, plat_probe_callback_fn);
	if (err) {
		pr_err("Probe platform driver failed\n");
		platform_device_unregister(pdev);
	}

	return err;
}

static void __exit exit_callback_fn(void)
{
	printk("Exiting plat1 driver \n");
	platform_device_unregister(pdev);
	platform_driver_unregister(&plat1_driver);
}


MODULE_AUTHOR("test");
MODULE_DESCRIPTION("plat test");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

module_init(init_callback_fn);
module_exit(exit_callback_fn);