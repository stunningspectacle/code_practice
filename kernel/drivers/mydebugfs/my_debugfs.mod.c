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
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x3b69997a, "module_layout" },
	{ 0xa8edc24f, "debugfs_remove_recursive" },
	{ 0xc8ae1a7c, "debugfs_create_x64" },
	{ 0x27e1a049, "printk" },
	{ 0x4fc7d84f, "debugfs_create_u64" },
	{ 0x76e14694, "debugfs_create_file" },
	{ 0x94263d2a, "debugfs_create_dir" },
	{ 0x5a5e7ea3, "simple_read_from_buffer" },
	{ 0xe0bc4fb2, "simple_write_to_buffer" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "71F57471B5CB887512419D1");
