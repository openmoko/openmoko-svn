#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x532018d2, "struct_module" },
	{ 0xa3c0f648, "SDLIB_PrintBuffer" },
	{ 0x4c3af445, "__request_region" },
	{ 0x7b615504, "pci_bus_read_config_byte" },
	{ 0x89b301d4, "param_get_int" },
	{ 0xdc3eaf70, "iomem_resource" },
	{ 0xf837928e, "malloc_sizes" },
	{ 0xdd7ad35b, "pci_disable_device" },
	{ 0x38b0c38f, "_spin_lock" },
	{ 0x54186cbc, "SDIO_RegisterHostController" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x90a8e6dd, "SDIO_CheckResponse" },
	{ 0xd533bec7, "__might_sleep" },
	{ 0x11067792, "__spin_lock_init" },
	{ 0x7d04a0f1, "SDIO_HandleHcdEvent" },
	{ 0xda4008e6, "cond_resched" },
	{ 0x1b7d4074, "printk" },
	{ 0xb029887b, "_spin_lock_irq" },
	{ 0x625acc81, "__down_failed_interruptible" },
	{ 0x90651ced, "_spin_unlock" },
	{ 0x29caedd7, "kmem_cache_alloc" },
	{ 0x3762cb6e, "ioremap_nocache" },
	{ 0xa596d99e, "schedule_delayed_work" },
	{ 0x2cf190e3, "request_irq" },
	{ 0x17d59d01, "schedule_timeout" },
	{ 0x8bb33e7d, "__release_region" },
	{ 0x3e378386, "pci_unregister_driver" },
	{ 0x9bde6acc, "init_waitqueue_head" },
	{ 0x768de002, "init_timer" },
	{ 0x1bdd6d27, "SDIO_UnregisterHostController" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e60bace, "memcpy" },
	{ 0x801678, "flush_scheduled_work" },
	{ 0xedc03953, "iounmap" },
	{ 0x75b19ede, "__pci_register_driver" },
	{ 0x60a4461c, "__up_wakeup" },
	{ 0x25da070, "snprintf" },
	{ 0xd958becb, "pci_enable_device" },
	{ 0x8391df7e, "_spin_unlock_irq" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=sdio_lib,sdio_busdriver";

MODULE_ALIAS("pci:v00001679d00003000sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v*d*sv*sd*bc08sc05i*");

MODULE_INFO(srcversion, "99791C6942B85A0E5ED808C");
