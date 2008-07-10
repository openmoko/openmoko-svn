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
	{ 0xfd7b24ba, "__mod_timer" },
	{ 0x89b301d4, "param_get_int" },
	{ 0xd467c828, "del_timer" },
	{ 0xf837928e, "malloc_sizes" },
	{ 0x7486345b, "SDLIB_GetMessage" },
	{ 0x425e773f, "SDLIB_PostMessage" },
	{ 0x38b0c38f, "_spin_lock" },
	{ 0xf32d2678, "schedule_work" },
	{ 0x589b2cf9, "SDLIB_IssueConfig" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x66c8bcd7, "_spin_lock_irqsave" },
	{ 0x7d11c268, "jiffies" },
	{ 0xd533bec7, "__might_sleep" },
	{ 0xf4cf19e6, "pnp_unregister_driver" },
	{ 0x11067792, "__spin_lock_init" },
	{ 0xda4008e6, "cond_resched" },
	{ 0x1b7d4074, "printk" },
	{ 0x2346dab4, "SDLIB_FindTuple" },
	{ 0x625acc81, "__down_failed_interruptible" },
	{ 0x9783f63f, "SDLIB_DeleteMessageQueue" },
	{ 0x8baed6ae, "_spin_unlock_irqrestore" },
	{ 0x13d8ce, "SDLIB_OSDeleteHelper" },
	{ 0x3e848ff6, "pnp_register_driver" },
	{ 0x90651ced, "_spin_unlock" },
	{ 0x9bed34a0, "module_put" },
	{ 0x978086d9, "SDLIB_OSCreateHelper" },
	{ 0x29caedd7, "kmem_cache_alloc" },
	{ 0x17d59d01, "schedule_timeout" },
	{ 0xca7800ab, "SDLIB_CreateMessageQueue" },
	{ 0x9bde6acc, "init_waitqueue_head" },
	{ 0x768de002, "init_timer" },
	{ 0x5401aa1d, "pnp_init_resource_table" },
	{ 0x37a0cba, "kfree" },
	{ 0x60a4461c, "__up_wakeup" },
	{ 0x25da070, "snprintf" },
	{ 0x96b27088, "__down_failed" },
	{ 0x586047e3, "SDLIB_IssueCMD52" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=sdio_lib";


MODULE_INFO(srcversion, "187072040D962892D6320C7");
