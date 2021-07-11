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
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xa879d70c, "module_layout" },
	{ 0xdc1aa928, "uart_unregister_driver" },
	{ 0xd9551df9, "uart_add_one_port" },
	{ 0x88679ed2, "uart_register_driver" },
	{ 0x68e44ccc, "uart_write_wakeup" },
	{ 0x5b8524d5, "tty_flip_buffer_push" },
	{ 0x132547f7, "__tty_insert_flip_char" },
	{ 0x9b67a295, "kmem_cache_alloc_trace" },
	{ 0x4b5317f6, "kmalloc_caches" },
	{ 0x24d273d1, "add_timer" },
	{ 0x15ba50a6, "jiffies" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0x2b68bd2f, "del_timer" },
	{ 0x6b2a0e17, "uart_get_divisor" },
	{ 0xe1b4e4b4, "uart_get_baud_rate" },
	{ 0x837b7b09, "__dynamic_pr_debug" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "419B67E891A9D7930A0D3DC");
