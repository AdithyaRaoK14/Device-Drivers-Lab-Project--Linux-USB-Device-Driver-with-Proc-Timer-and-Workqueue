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

KSYMTAB_FUNC(ddk_ext_on_write_success, "", "");
KSYMTAB_FUNC(ddk_ext_on_write_fail, "", "");
KSYMTAB_FUNC(ddk_ext_init, "", "");
KSYMTAB_FUNC(ddk_ext_cleanup, "", "");

SYMBOL_CRC(ddk_ext_on_write_success, 0xf1d85422, "");
SYMBOL_CRC(ddk_ext_on_write_fail, 0x88758311, "");
SYMBOL_CRC(ddk_ext_init, 0x7915626a, "");
SYMBOL_CRC(ddk_ext_cleanup, 0x88758311, "");

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xa61fd7aa, "__check_object_size" },
	{ 0x092a35a2, "_copy_from_user" },
	{ 0x534ed5f3, "__msecs_to_jiffies" },
	{ 0xd710adbf, "__kmalloc_noprof" },
	{ 0x49733ad6, "queue_work_on" },
	{ 0xadb55ac9, "usb_register_driver" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0x253f0c1d, "seq_lseek" },
	{ 0x5452ea08, "proc_create_data" },
	{ 0x2352b148, "timer_delete" },
	{ 0x2352b148, "timer_delete_sync" },
	{ 0xe1e1f979, "_raw_spin_lock_irqsave" },
	{ 0xd272d446, "__fentry__" },
	{ 0xf7064e45, "usb_interrupt_msg" },
	{ 0x9d783e76, "usb_find_interface" },
	{ 0xe8213e80, "_printk" },
	{ 0xbd03ed67, "__ref_stack_chk_guard" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0x8eb863b6, "ddk_led_ctrl" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0x32feeafc, "mod_timer" },
	{ 0xbd03ed67, "random_kmalloc_seed" },
	{ 0xf46d5bf3, "mutex_lock" },
	{ 0x6c00f410, "usb_control_msg" },
	{ 0x3d212d3d, "usb_register_dev" },
	{ 0x173ec8da, "sscanf" },
	{ 0xc1e6c71e, "__mutex_init" },
	{ 0xa8f96c6e, "usb_deregister" },
	{ 0x81a1a811, "_raw_spin_unlock_irqrestore" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0x058c185a, "jiffies" },
	{ 0xaa9a3b35, "seq_read" },
	{ 0xf46d5bf3, "mutex_unlock" },
	{ 0xc0f19660, "remove_proc_entry" },
	{ 0xc064623f, "__kmalloc_cache_noprof" },
	{ 0x887666e1, "usb_deregister_dev" },
	{ 0x2d88a3ab, "cancel_work_sync" },
	{ 0x546c19d9, "validate_usercopy_range" },
	{ 0xb61837ba, "seq_printf" },
	{ 0x34d5450c, "single_release" },
	{ 0x02f9bbf0, "timer_init_key" },
	{ 0xe4de56b4, "__ubsan_handle_load_invalid_value" },
	{ 0xe931a49e, "single_open" },
	{ 0x67628f51, "msleep" },
	{ 0xfaabfe5e, "kmalloc_caches" },
	{ 0xaef1f20d, "system_wq" },
	{ 0xbebe66ff, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xa61fd7aa,
	0x092a35a2,
	0x534ed5f3,
	0xd710adbf,
	0x49733ad6,
	0xadb55ac9,
	0xcb8b6ec6,
	0x253f0c1d,
	0x5452ea08,
	0x2352b148,
	0x2352b148,
	0xe1e1f979,
	0xd272d446,
	0xf7064e45,
	0x9d783e76,
	0xe8213e80,
	0xbd03ed67,
	0xd272d446,
	0x8eb863b6,
	0x90a48d82,
	0x32feeafc,
	0xbd03ed67,
	0xf46d5bf3,
	0x6c00f410,
	0x3d212d3d,
	0x173ec8da,
	0xc1e6c71e,
	0xa8f96c6e,
	0x81a1a811,
	0xd272d446,
	0x092a35a2,
	0x058c185a,
	0xaa9a3b35,
	0xf46d5bf3,
	0xc0f19660,
	0xc064623f,
	0x887666e1,
	0x2d88a3ab,
	0x546c19d9,
	0xb61837ba,
	0x34d5450c,
	0x02f9bbf0,
	0xe4de56b4,
	0xe931a49e,
	0x67628f51,
	0xfaabfe5e,
	0xaef1f20d,
	0xbebe66ff,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"__check_object_size\0"
	"_copy_from_user\0"
	"__msecs_to_jiffies\0"
	"__kmalloc_noprof\0"
	"queue_work_on\0"
	"usb_register_driver\0"
	"kfree\0"
	"seq_lseek\0"
	"proc_create_data\0"
	"timer_delete\0"
	"timer_delete_sync\0"
	"_raw_spin_lock_irqsave\0"
	"__fentry__\0"
	"usb_interrupt_msg\0"
	"usb_find_interface\0"
	"_printk\0"
	"__ref_stack_chk_guard\0"
	"__stack_chk_fail\0"
	"ddk_led_ctrl\0"
	"__ubsan_handle_out_of_bounds\0"
	"mod_timer\0"
	"random_kmalloc_seed\0"
	"mutex_lock\0"
	"usb_control_msg\0"
	"usb_register_dev\0"
	"sscanf\0"
	"__mutex_init\0"
	"usb_deregister\0"
	"_raw_spin_unlock_irqrestore\0"
	"__x86_return_thunk\0"
	"_copy_to_user\0"
	"jiffies\0"
	"seq_read\0"
	"mutex_unlock\0"
	"remove_proc_entry\0"
	"__kmalloc_cache_noprof\0"
	"usb_deregister_dev\0"
	"cancel_work_sync\0"
	"validate_usercopy_range\0"
	"seq_printf\0"
	"single_release\0"
	"timer_init_key\0"
	"__ubsan_handle_load_invalid_value\0"
	"single_open\0"
	"msleep\0"
	"kmalloc_caches\0"
	"system_wq\0"
	"module_layout\0"
;

MODULE_INFO(depends, "ddk_led");

MODULE_ALIAS("usb:v16C0p05DCd*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "798676B25F941A7DA52EA8D");
