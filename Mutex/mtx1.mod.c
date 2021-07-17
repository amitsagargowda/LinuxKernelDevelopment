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
	{ 0xb22e7a5b, "kthread_stop" },
	{ 0x2af57f44, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x111bd0bd, "class_destroy" },
	{ 0x374f9f, "wake_up_process" },
	{ 0x629d43f8, "kthread_create_on_node" },
	{ 0x977f511b, "__mutex_init" },
	{ 0x213ef32e, "device_create" },
	{ 0x6c496836, "__class_create" },
	{ 0x4bf89aa2, "cdev_add" },
	{ 0xc875eb96, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xf9a482f9, "msleep" },
	{ 0x409bcb62, "mutex_unlock" },
	{ 0x2ab7989d, "mutex_lock" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "28483E6466F4C88C983F1BF");
