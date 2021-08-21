#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/mutex.h>
#include <linux/errno.h>
#include <linux/slab.h>

#include <asm/uaccess.h>

#define TEST_USB1_VENDOR_ID 0x0781
#define TEST_USB1_PRODUCT_ID 0x5581
#define EP_IN (USB_DIR_IN | 0x01)
#define EP_OUT 0x01

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

struct test_usb1_device
{
	struct mutex m;
	int in_ep, out_ep;
	int in_buf_size, out_buf_size;
	unsigned char *in_buf, *out_buf;
	int in_buf_left_over;
	struct usb_device *device;
	struct usb_class_driver ucd;
};

static struct usb_driver test_usb1_driver;

static int test_usb1_open(struct inode *i, struct file *f)
{
	struct usb_interface *interface;
	struct test_usb1_device *dev;

	interface = usb_find_interface(&test_usb1_driver, iminor(i));
	if (!interface)
	{
		printk(KERN_ERR "%s - error, can't find device for minor %d\n",
				__func__, iminor(i));
		return -ENODEV;
	}   

	if (!(dev = usb_get_intfdata(interface)))
	{
		return -ENODEV;
	}

	if (!mutex_trylock(&dev->m))
	{
		return -EBUSY;
	}

	dev->in_buf_left_over = 0;
	f->private_data = (void *)(dev);
	return 0;
}
static int test_usb1_close(struct inode *i, struct file *f)
{
	struct test_usb1_device *dev = (struct test_usb1_device *)(f->private_data);

	mutex_unlock(&dev->m);
	return 0;
}
static ssize_t test_usb1_write(struct file *f, const char __user *buf, size_t cnt, loff_t *off)
{
	struct test_usb1_device *dev = (struct test_usb1_device *)(f->private_data);
	int write_size, wrote_size, wrote_cnt;
	int retval;

	wrote_cnt = 0;
	while (wrote_cnt < cnt)
	{
		write_size = MIN(dev->out_buf_size, cnt - wrote_cnt /* Remaining */);
		/* Using out_buf may cause sync issues */
		if (copy_from_user(dev->out_buf, buf + wrote_cnt, write_size))
		{
			return -EFAULT;
		}
		/* Send the data out of the bulk endpoint */
		retval = usb_bulk_msg(dev->device, usb_sndbulkpipe(dev->device, dev->out_ep),
			dev->out_buf, write_size, &wrote_size, 10);
		if (retval < 0)
		{
			printk(KERN_ERR "Bulk message returned %d\n", retval);
			return retval;
		}
		wrote_cnt += wrote_size;
		printk(KERN_INFO "Wrote %d bytes\n", wrote_size);
	}

	return wrote_cnt;
}
static ssize_t test_usb1_read(struct file *f, char __user *buf, size_t cnt, loff_t *off)
{
	struct test_usb1_device *dev = (struct test_usb1_device *)(f->private_data);
	int read_size, read_cnt;
	int retval;
	int i;

	printk(KERN_INFO "Read request for %zd bytes\n", cnt);
	/* Check for left over data */
	if (dev->in_buf_left_over)
	{
		read_size = dev->in_buf_left_over;
		dev->in_buf_left_over = 0;
	}
	else
	{
		/* Read the data in from the bulk endpoint */
		/* Using in_buf may cause sync issues - need mutex protection */
		retval = usb_bulk_msg(dev->device, usb_rcvbulkpipe(dev->device, dev->in_ep),
			dev->in_buf, dev->in_buf_size, &read_size, 10);
		if (retval < 0)
		{
			printk(KERN_ERR "Bulk message returned %d\n", retval);
#if 0
			return retval;
#else
			if (retval != -ETIMEDOUT)
			{
				return retval;
			}
			else /* Not really an error but no data available */
			{
				read_size = 0;
			}
#endif
		}
	}
	if (read_size <= cnt)
	{
		read_cnt = read_size;
	}
	else
	{
		read_cnt = cnt;
	}
	if (copy_to_user(buf, dev->in_buf, read_cnt))
	{
		dev->in_buf_left_over = read_size;
		return -EFAULT;
	}
	for (i = cnt; i < read_size; i++)
	{
		dev->in_buf[i - cnt] = dev->in_buf[i];
	}
	if (cnt < read_size)
	{
		dev->in_buf_left_over = read_size - cnt;
	}
	else
	{
		dev->in_buf_left_over = 0;
	}
	printk(KERN_INFO "Actually read %d bytes (Sent to user: %d)\n", read_size, read_cnt);

	return read_cnt;
}

static struct file_operations fops =
{
	.owner = THIS_MODULE,
	.open = test_usb1_open,
	.release = test_usb1_close,
	.write = test_usb1_write,
	.read = test_usb1_read,
};

static int test_usb1_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct test_usb1_device *test_usb1_dev;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	int i;
	int retval;

	iface_desc = interface->cur_altsetting;
	printk(KERN_INFO "TEST USB1 i/f %d now probed: (%04X:%04X)\n",
			iface_desc->desc.bInterfaceNumber,
			id->idVendor, id->idProduct);

	test_usb1_dev = (struct test_usb1_device *)(kzalloc(sizeof(struct test_usb1_device), GFP_KERNEL));
	if (!test_usb1_dev)
	{
		printk(KERN_ERR "Not able to get memory for data structure of this device.\n");
		retval = -ENOMEM;
		goto probe_err;
	}
	mutex_init(&test_usb1_dev->m);

	/* Set up the endpoint information related to serial */
	for (i = 0; i < iface_desc->desc.bNumEndpoints; i++)
	{
		endpoint = &iface_desc->endpoint[i].desc;

		if (endpoint->bEndpointAddress == EP_IN)
		{
			test_usb1_dev->in_ep = endpoint->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
			test_usb1_dev->in_buf_size = endpoint->wMaxPacketSize;
			test_usb1_dev->in_buf = kmalloc(endpoint->wMaxPacketSize, GFP_KERNEL);
			if (!test_usb1_dev->in_buf)
			{
				printk(KERN_ERR "Not able to get memory for in ep buffer of this device.\n");
				retval = -ENOMEM;
				goto probe_err;
			}
			test_usb1_dev->in_buf_left_over = 0;
		}
		if (endpoint->bEndpointAddress == EP_OUT)
		{
			test_usb1_dev->out_ep = endpoint->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
			test_usb1_dev->out_buf_size = endpoint->wMaxPacketSize;
			test_usb1_dev->out_buf = kmalloc(endpoint->wMaxPacketSize, GFP_KERNEL);
			if (!test_usb1_dev->out_buf)
			{
				printk(KERN_ERR "Not able to get memory for out ep buffer of this device.\n");
				retval = -ENOMEM;
				goto probe_err;
			}
		}
	}

	test_usb1_dev->device = interface_to_usbdev(interface);

	test_usb1_dev->ucd.name = "test_usb1%d";
	test_usb1_dev->ucd.fops = &fops;
	retval = usb_register_dev(interface, &test_usb1_dev->ucd);
	if (retval)
	{
		/* Something prevented us from registering this driver */
		printk(KERN_ERR "Not able to get a minor for this device.\n");
		goto probe_err;
	}
	else
	{
		printk(KERN_INFO "Minor obtained: %d\n", interface->minor);
	}

	usb_set_intfdata(interface, test_usb1_dev);

	return 0;

probe_err:
	if (test_usb1_dev->out_buf)
	{
		kfree(test_usb1_dev->out_buf);
	}
	if (test_usb1_dev->in_buf)
	{
		kfree(test_usb1_dev->in_buf);
	}
	if (test_usb1_dev)
	{
		kfree(test_usb1_dev);
	}

	return retval;
}

static void test_usb1_disconnect(struct usb_interface *interface)
{
	struct test_usb1_device *test_usb1_dev;

	printk(KERN_INFO "Releasing Minor: %d\n", interface->minor);

	test_usb1_dev = (struct test_usb1_device *)(usb_get_intfdata(interface));
	usb_set_intfdata(interface, NULL);

	/* Give back our minor */
	usb_deregister_dev(interface, &test_usb1_dev->ucd);

	/* Prevent test_usb1_open() from racing test_usb1_disconnect() */
	mutex_lock(&test_usb1_dev->m);
	kfree(test_usb1_dev->out_buf);
	kfree(test_usb1_dev->in_buf);
	mutex_unlock(&test_usb1_dev->m);

	kfree(test_usb1_dev);

	printk(KERN_INFO "TEST USB1 i/f %d now disconnected\n",
			interface->cur_altsetting->desc.bInterfaceNumber);
}

/* Table of devices that work with this driver */
static struct usb_device_id test_usb1_table[] =
{
	{
		USB_DEVICE(TEST_USB1_VENDOR_ID, TEST_USB1_PRODUCT_ID)
	},
	{} /* Terminating entry */
};
MODULE_DEVICE_TABLE (usb, test_usb1_table);

static struct usb_driver test_usb1_driver =
{
	.name = "test_usb1_bulk",
	.probe = test_usb1_probe,
	.disconnect = test_usb1_disconnect,
	.id_table = test_usb1_table,
};

static int __init test_usb1_init(void)
{
	int result;

	/* Register this driver with the USB subsystem */
	if ((result = usb_register(&test_usb1_driver)))
	{
		printk(KERN_ERR "usb_register failed. Error number %d\n", result);
	}
	printk(KERN_INFO "TEST USB1 usb_registered\n");
	return result;
}

static void __exit test_usb1_exit(void)
{
	/* Deregister this driver with the USB subsystem */
	usb_deregister(&test_usb1_driver);
	printk(KERN_INFO "TEST USB1 usb_deregistered\n");
}

module_init(test_usb1_init);
module_exit(test_usb1_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("test");
MODULE_DESCRIPTION("USB Host Driver for TEST USB1 Loop Back Bulk Device");
