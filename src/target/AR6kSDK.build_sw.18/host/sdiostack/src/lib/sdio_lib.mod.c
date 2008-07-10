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
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xa738d1cc, "complete_and_exit" },
	{ 0x89b301d4, "param_get_int" },
	{ 0xf837928e, "malloc_sizes" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x66c8bcd7, "_spin_lock_irqsave" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x11067792, "__spin_lock_init" },
	{ 0x40eb6ac6, "wait_for_completion" },
	{ 0x1b7d4074, "printk" },
	{ 0x8baed6ae, "_spin_unlock_irqrestore" },
	{ 0x29caedd7, "kmem_cache_alloc" },
	{ 0xb8ed13e6, "wake_up_process" },
	{ 0x9bde6acc, "init_waitqueue_head" },
	{ 0x37a0cba, "kfree" },
	{ 0xdd1e87ae, "kthread_create" },
	{ 0x60a4461c, "__up_wakeup" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "3F23A6C8D3AE082934D002E");
