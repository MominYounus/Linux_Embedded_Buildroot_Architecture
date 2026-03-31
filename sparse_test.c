#include <linux/fs.h>
#include <linux/module.h>
#include <linux/uaccess.h>

static long bad_ioctl(struct file *f, unsigned int cmd, unsigned long arg) {
    int *kptr = (int *)arg;
    *kptr = 42;
    return 0;
}

MODULE_LICENSE("GPL");
