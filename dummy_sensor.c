#include <linux/fs.h>         // Added for vfs
#include <linux/interrupt.h>  // For IRQ API's
#include <linux/miscdevice.h> // Added for /dev node
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h> // copy_to_user
#include <linux/minmax.h> // partial read

// Global variable to hold our hardware data
static u32 active_sensor_id = 0;
static int sample_rate = 50;

static irqreturn_t sensor_isr(int irq, void *dev_id) {
    printk(KERN_INFO "DummySensor: INTERRUPT FIRED on IRQ %d!\n", irq);
    return IRQ_HANDLED;
}

//--- PHASE 1: USER SPACE INTERFACE ---
// Partial Read
static ssize_t sensor_read(struct file *file, char __user *user_buf,
						   size_t count, loff_t *ppos) {
	char buf[32];
	int len;
	int bytes_to_copy;

	if (*ppos == 0)
		active_sensor_id = 0x42;
	else
		active_sensor_id = 0x99;
	
	len = snprintf(buf, sizeof(buf), "Hardware Sensor ID is: 0x%X\n", active_sensor_id);

	if (*ppos >= len)
		return 0;

	bytes_to_copy = min((size_t)count, (size_t)len - *ppos);

	if (copy_to_user(user_buf, buf + *ppos, bytes_to_copy))
		return -EFAULT;

	*ppos += bytes_to_copy;
	return bytes_to_copy;
	
}

static const struct file_operations sensor_fops = {
    .owner = THIS_MODULE,
    .read = sensor_read,
};

static struct miscdevice sensor_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "dummy_sensor",
    .fops = &sensor_fops,
};

// --- PHASE 2: THE HARDWARE INTERFACE ---

/* sysfs control panel */

// called when the user runs: cat /sys/.../sample_rate
static ssize_t sample_rate_show(struct device *dev,
                                struct device_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", sample_rate);
}

// Called when the user runs: echo 100 > /sys/.../sample_rate
static ssize_t sample_rate_store(struct device *dev,
                                 struct device_attribute *attr, const char *buf,
                                 size_t count) {
	int new_rate;
	// Parse the string from user space into an integer
	if(sscanf(buf, "%d", &new_rate) == 1) {
		sample_rate = new_rate;
		printk(KERN_INFO "DummySensor: Sample rate changed to %d Hz\n", sample_rate);
	}
	return count;
}

// To create a dev_attr_sample_rate struct automatically
static DEVICE_ATTR_RW(sample_rate);

/* PROBE FUNCTION */
static int sensor_probe(struct platform_device *pdev) {
    struct device *dev = &pdev->dev;
    int ret;
    int irq_num;

    printk(KERN_INFO "DummySensor: Hardware found. Probing...\n");

    ret = of_property_read_u32(dev->of_node, "sensor-id", &active_sensor_id);
    if (ret)
        return ret;

    irq_num = platform_get_irq(pdev, 0);
    if (irq_num < 0) {
        printk(KERN_ERR "DummySensor: No IRQ found in DTB\n");
        return irq_num;
    }
    printk(KERN_INFO "DummySensor: Mapped to Hardware IRQ %d\n", irq_num);

    ret =
        devm_request_irq(dev, irq_num, sensor_isr, 0, "dummy_sensor_irq", NULL);
    if (ret) {
        printk(KERN_ERR "DummySensor: Failed to request IRQ %d\n", irq_num);
        return ret;
    }

    // Extracting the custom property from the DEVICE TREE
    ret = misc_register(&sensor_misc);
    if (ret) {
        printk(KERN_INFO "DummySensor: Failed to create /dev node\n");
        return ret;
    }

    printk(KERN_INFO "DummySensor: PROBE Complete\n");

	ret = sysfs_create_file(&dev->kobj, &dev_attr_sample_rate.attr);
	if(ret) {
		printk(KERN_INFO "DummySensor: Failed to create sysfs entry\n");
		return ret;
	}
	
    return 0;
}

/* ROOM FUNCTION */
// Called when the module is unloaded or physically ripped out
static int sensor_remove(struct platform_device *pdev) {
    misc_deregister(&sensor_misc);
    printk(KERN_INFO "DummySensor: Driver and /dev node removed.\n");

	sysfs_remove_file(&pdev->dev.kobj, &dev_attr_sample_rate.attr);
	
    return 0;
}

/* 1. MATCH TABLE */
// This tells the kernel exactly what hardware we are looking for
static const struct of_device_id sensor_dt_match[] = {
    {.compatible = "kernelmentor,dummy-sensor"},
    {/* Sentinel (must be NULL terminated)  */},
};
MODULE_DEVICE_TABLE(of, sensor_dt_match);

/* 2. THE DRIVER STRUCTURE */
static struct platform_driver sensor_driver = {
    .probe = sensor_probe,
    .remove = sensor_remove,
    .driver =
        {
            .name = "dummy_sensor_driver",
            .of_match_table =
                sensor_dt_match, // connecting the match table sensor_dt_match
        },
};

/* 5. SHORTCUT FOR module_init() and module_exit() */
// this replaces the module_init() and module_exit()
module_platform_driver(sensor_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("KERNEL_MENTOR");
MODULE_DESCRIPTION("A simple Platform Driver testing DTB matching");
