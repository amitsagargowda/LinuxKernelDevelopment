#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xa879d70c, "module_layout" },
	{ 0x7adc5eab, "device_destroy" },
	{ 0x2af57f44, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x111bd0bd, "class_destroy" },
	{ 0xabdd13a9, "sysfs_remove_file_ns" },
	{ 0xf4c6a598, "kobject_put" },
	{ 0xc1514a3b, "free_irq" },
	{ 0x2072ee9b, "request_threaded_irq" },
	{ 0xa419798f, "sysfs_create_file_ns" },
	{ 0x8318cef0, "kobject_create_and_add" },
	{ 0x2ac0c16c, "kernel_kobj" },
	{ 0x213ef32e, "device_create" },
	{ 0x6c496836, "__class_create" },
	{ 0x4bf89aa2, "cdev_add" },
	{ 0xc875eb96, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0xbcab6ee6, "sscanf" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "CE3E00ED9F5C7AC19B9F949");
