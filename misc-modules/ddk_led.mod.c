#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

KSYMTAB_FUNC(ddk_led_ctrl, "", "");

SYMBOL_CRC(ddk_led_ctrl, 0x8eb863b6, "");

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x887666e1, "usb_deregister_dev" },
	{ 0x3d212d3d, "usb_register_dev" },
	{ 0xfaabfe5e, "kmalloc_caches" },
	{ 0xbd03ed67, "__ref_stack_chk_guard" },
	{ 0xc064623f, "__kmalloc_cache_noprof" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0xa8f96c6e, "usb_deregister" },
	{ 0xd272d446, "__fentry__" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x6c00f410, "usb_control_msg" },
	{ 0xadb55ac9, "usb_register_driver" },
	{ 0xe8213e80, "_printk" },
	{ 0xbebe66ff, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x887666e1,
	0x3d212d3d,
	0xfaabfe5e,
	0xbd03ed67,
	0xc064623f,
	0x092a35a2,
	0xd272d446,
	0xa8f96c6e,
	0xd272d446,
	0xd272d446,
	0x6c00f410,
	0xadb55ac9,
	0xe8213e80,
	0xbebe66ff,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"usb_deregister_dev\0"
	"usb_register_dev\0"
	"kmalloc_caches\0"
	"__ref_stack_chk_guard\0"
	"__kmalloc_cache_noprof\0"
	"_copy_to_user\0"
	"__stack_chk_fail\0"
	"usb_deregister\0"
	"__fentry__\0"
	"__x86_return_thunk\0"
	"usb_control_msg\0"
	"usb_register_driver\0"
	"_printk\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");

MODULE_ALIAS("usb:v16C0p05DCd*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "ADB08B0CC874311E0638C64");
