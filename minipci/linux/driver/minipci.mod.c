#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x30ff7695, "module_layout" },
	{ 0x11eb121f, "cdev_del" },
	{ 0xd90cd7e6, "cdev_init" },
	{ 0x46cf10eb, "cachemode2protval" },
	{ 0xcb2f2b52, "boot_cpu_data" },
	{ 0x3d258838, "pci_disable_device" },
	{ 0xc85ac280, "device_destroy" },
	{ 0x145ca79, "pci_release_regions" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xa648e561, "__ubsan_handle_shift_out_of_bounds" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x904354ef, "pci_iounmap" },
	{ 0x4c9d28b0, "phys_base" },
	{ 0x9befafec, "device_create" },
	{ 0x3d519628, "_dev_err" },
	{ 0x646eac6, "cdev_add" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0x2d9100d8, "module_put" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x92997ed8, "_printk" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xc0b2bc48, "pci_unregister_driver" },
	{ 0xdb8acb90, "remap_pfn_range" },
	{ 0x210cdac4, "pci_request_regions" },
	{ 0x433f0b06, "__pci_register_driver" },
	{ 0xb3f0559, "class_destroy" },
	{ 0xd6928da2, "pci_iomap" },
	{ 0x58a78df9, "pci_enable_device" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x52ea150d, "__class_create" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x60bd478a, "try_module_get" },
	{ 0x8a35b432, "sme_me_mask" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "FFF423217045532B91D39DE");
