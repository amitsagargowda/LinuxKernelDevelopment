#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

#define TEST_USB_VENDOR_ID 0x0781
#define TEST_USB_PRODUCT_ID 0x5581
static struct usb_class_driver ucd;

static int test_usb_open(struct inode *i, struct file *f)
{
	return 0;
}
static int test_usb_close(struct inode *i, struct file *f)
{
	return 0;
}

static struct file_operations test_usb_fops =
{
	.owner = THIS_MODULE,
	.open = test_usb_open,
	.release = test_usb_close,
};

static int test_usb_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	int i;
	int retval;

	iface_desc = interface->cur_altsetting;

	printk(KERN_INFO "TEST USB i/f %d now probed: (%04X:%04X)\n",
			iface_desc->desc.bInterfaceNumber, id->idVendor, id->idProduct);
	printk(KERN_INFO "ID->bNumEndpoints: %02X\n",
			iface_desc->desc.bNumEndpoints);
	printk(KERN_INFO "ID->bInterfaceClass: %02X\n",
			iface_desc->desc.bInterfaceClass);

	for (i = 0; i < iface_desc->desc.bNumEndpoints; i++)
	{
		endpoint = &iface_desc->endpoint[i].desc;

		printk(KERN_INFO "ED[%d]->bEndpointAddress: 0x%02X\n", i,
				endpoint->bEndpointAddress);
		printk(KERN_INFO "ED[%d]->bmAttributes: 0x%02X\n", i,
				endpoint->bmAttributes);
		printk(KERN_INFO "ED[%d]->wMaxPacketSize: 0x%04X (%d)\n", i,
				endpoint->wMaxPacketSize, endpoint->wMaxPacketSize);
	}

	{
		ucd.name = "test_usb%d";
		ucd.fops = &test_usb_fops;
		retval = usb_register_dev(interface, &ucd);
		if (retval)
		{
			/* Something prevented us from registering this driver */
			printk(KERN_ERR "Not able to get a minor for this device.\n");
			return retval;
		}
		else
		{
			printk(KERN_INFO "Minor obtained: %d\n", interface->minor);
		}
	}

	return 0;
}

static void test_usb_disconnect(struct usb_interface *interface)
{
	{
		printk(KERN_INFO "Releasing Minor: %d\n", interface->minor);
		usb_deregister_dev(interface, &ucd);
	}

	printk(KERN_INFO "TEST USB i/f %d now disconnected\n",
			interface->cur_altsetting->desc.bInterfaceNumber);
}

/* Table of devices that work with this driver */
static struct usb_device_id test_usb_table[] =
{
	{
		USB_DEVICE(TEST_USB_VENDOR_ID, TEST_USB_PRODUCT_ID)
	},
	{} /* Terminating entry */
};
MODULE_DEVICE_TABLE (usb, test_usb_table);

static struct usb_driver test_usb_driver =
{
	.name = "test_usb_vert",
	.probe = test_usb_probe,
	.disconnect = test_usb_disconnect,
	.id_table = test_usb_table,
};

static int __init test_usb_init(void)
{
	int result;

	/* Register this driver with the USB subsystem */
	if ((result = usb_register(&test_usb_driver)))
	{
		printk(KERN_ERR "usb_register failed. Error number %d\n", result);
	}
	printk(KERN_INFO "TEST usb_registered\n");
	return result;
}

static void __exit test_usb_exit(void)
{
	/* Deregister this driver with the USB subsystem */
	usb_deregister(&test_usb_driver);
	printk(KERN_INFO "TEST USB usb_deregistered\n");
}

module_init(test_usb_init);
module_exit(test_usb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Test");
MODULE_DESCRIPTION("Test USB Driver");
