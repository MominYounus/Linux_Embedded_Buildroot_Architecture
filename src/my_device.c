#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h> //For File OPeration
#include <linux/miscdevice.h>//for misc_register
#include <linux/uaccess.h> //For copy_to/from_user
#include <linux/ioctl.h> //Input/Output Control
#include <linux/mutex.h>
#include <linux/timer.h>
#include <linux/jiffies.h> //To prevent wastage of CPU cycles

#define MY_IOCTL_CLEAR_BUFFER _IO('k', 1)

static char kernel_buffer[256]; //A place to store data
static int buffer_len = 0;
DEFINE_MUTEX(my_mutex);

/* jiffy timer */
/*
 * Current time = jiffies
 * One Second from now = jiffies + HZ
 * Half a second from now = jiffies + (HZ / 2)
 */
static struct timer_list my_timer;

/* called when user reads: cat /dev/my_device */
static ssize_t my_read(struct file *file, char __user *user_buf, size_t count, loff_t *popps)
{
	int bytes_to_copy;
	int ret;

	//if end of the buffer return 0 (EOF)
	if (*popps >= buffer_len) return 0;

	//caluclate how to copy
	bytes_to_copy = buffer_len - *popps;
	if (bytes_to_copy > count) bytes_to_copy = count;

	/* Using mutex to avoid Race Conditions. */
	mutex_lock(&my_mutex);

	if (copy_to_user(user_buf, kernel_buffer + *popps, bytes_to_copy)) {
		ret = -EFAULT; //failed to copy
		goto unlock_and_exit;
	}
	
	*popps += bytes_to_copy;
	ret = bytes_to_copy;

 unlock_and_exit:
	mutex_unlock(&my_mutex);
	return ret;
}

/* called when user writes echo "hello" > /dev/my_device */
static ssize_t my_write(struct file *file, const char __user *user_buf, size_t count, loff_t *popps)
{
	int bytes_to_copy = count;
	int ret = count;
	
	//Don't overflow our static buffer
	if (bytes_to_copy > sizeof(kernel_buffer) - 1)
		bytes_to_copy = sizeof(kernel_buffer) - 1;

	mutex_lock(&my_mutex);
	
	//Copy data from user space
	if (copy_from_user(kernel_buffer, user_buf, bytes_to_copy)) {
		ret = -EFAULT;
		goto unlock_and_exit;
	}

	kernel_buffer[bytes_to_copy] = '\0';
	buffer_len = bytes_to_copy;
	
	printk(KERN_INFO "My device: Received %s\n", kernel_buffer);

 unlock_and_exit:
	mutex_unlock(&my_mutex);
	return ret; //return count on success, -EFAULT on fail
}

/* NOTE
 * If we are writing a driver to UART we can't use write() we need ioctl, though I heard ioctl is
 * losly packed and kernel devs avoid abusing it, but this is only for me learning
 * about kernel and device drivers.
 * Now this should clear the kernel buffer, without using write()
 */

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	mutex_lock(&my_mutex);
	
	switch(cmd) {
	case MY_IOCTL_CLEAR_BUFFER:
		printk(KERN_INFO "Mydevice: IOCTL commanded buffer clear\n");
		buffer_len = 0;
		break;

	default:
		ret = -ENOTTY;
		break;
	}

	mutex_unlock(&my_mutex);
	return ret;
}

static void timer_callback(struct timer_list *t)
{
	printk(KERN_INFO "MyDevice: Timer expired! Jiffies: %lu\n", jiffies);

	//Re-arm the timer to fire again in 2 seconds (repeating timer)
	mod_timer(&my_timer, jiffies + msecs_to_jiffies(2000));
}

/* mapping the functions to file operations */
static const struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.read = my_read,
	.write = my_write,
	.unlocked_ioctl = my_ioctl,
};

/* Define the device struct */
static struct miscdevice my_device = {
	.minor = MISC_DYNAMIC_MINOR, //let the KERNEL pick the number
	.name = "simple_device", //this name in /dev
	.fops = &my_fops,
};

static int __init my_init(void)
{
	int ret;
	ret = misc_register(&my_device); //this returns 0 if SUCCESS and a -ve number if FAILED. 

	if (ret)
		return ret;

	printk(KERN_INFO "MyDevice: Registered /dev/simple_device\n");

	//Initializing the timer and linking it to the callback
	timer_setup(&my_timer, timer_callback, 0);

	mod_timer(&my_timer, jiffies + msecs_to_jiffies(2000));
	
	return 0;
}

static void __exit my_exit(void)
{
	del_timer(&my_timer);
	misc_deregister(&my_device);
	printk(KERN_INFO "MyDevice: Unregistered......\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
