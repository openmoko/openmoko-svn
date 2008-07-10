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
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xed27bb9f, "mem_map" },
	{ 0x89b301d4, "param_get_int" },
	{ 0xdc3eaf70, "iomem_resource" },
	{ 0xe1b7029c, "print_tainted" },
	{ 0xf837928e, "malloc_sizes" },
	{ 0x38b0c38f, "_spin_lock" },
	{ 0x54186cbc, "SDIO_RegisterHostController" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x90a8e6dd, "SDIO_CheckResponse" },
	{ 0x11067792, "__spin_lock_init" },
	{ 0x7d04a0f1, "SDIO_HandleHcdEvent" },
	{ 0x1b7d4074, "printk" },
	{ 0xb029887b, "_spin_lock_irq" },
	{ 0xd1f2bae4, "dma_free_coherent" },
	{ 0x90651ced, "_spin_unlock" },
	{ 0xb2ef7392, "dma_alloc_coherent" },
	{ 0x29caedd7, "kmem_cache_alloc" },
	{ 0x3762cb6e, "ioremap_nocache" },
	{ 0xa596d99e, "schedule_delayed_work" },
	{ 0x2cf190e3, "request_irq" },
	{ 0x17d59d01, "schedule_timeout" },
	{ 0x6b2dc060, "dump_stack" },
	{ 0x8bb33e7d, "__release_region" },
	{ 0x3e378386, "pci_unregister_driver" },
	{ 0x768de002, "init_timer" },
	{ 0x1bdd6d27, "SDIO_UnregisterHostController" },
	{ 0x37a0cba, "kfree" },
	{ 0x801678, "flush_scheduled_work" },
	{ 0xedc03953, "iounmap" },
	{ 0x75b19ede, "__pci_register_driver" },
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
MODULE_ALIAS("pci:v000010EEd00000300sv000010B5sd00009030bc*sc*i*");
MODULE_ALIAS("pci:v00001524d00000750sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v*d*sv*sd*bc08sc05i*");

MODULE_INFO(srcversion, "BD047B127B93B97127C299A");
