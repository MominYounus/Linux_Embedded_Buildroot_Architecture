#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("KERNEL MENTOR");
MODULE_DESCRIPTION("A Simple Hello World LKM");
MODULE_VERSION("0.1");

static int __init hello_start(void) {
	printk(KERN_INFO "Kernel Mentor: I am inside the KERNEL!\n");
	return 0;
}

static void __exit hello_end(void) {
	printk(KERN_INFO "kernel Mentor: Goodbye, leaving kernel space.\n");
}

module_init(hello_start);
module_exit(hello_end);
