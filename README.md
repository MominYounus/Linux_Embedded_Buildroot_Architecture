# Dummy Sensor Platform Driver

This is an complete, end-to-end Linux Platform Driver written for an ARM64 architecture.

## Features:
* **Device Tree Integration** Binds to a custom `Kernelmentor,dummy-sensor` node.
* **IRQ Handling** Maps and registers hardware interrupts using GIC.
* **VFS Data Pipeline** Exposes a Character Device node (`/dev/dummy-sensor`) for user-space data reads.
* **Sysfs Control Panel** Exposes a configurable `sample_rate` attribute in `/sys/bus/platform/devices`.

## Architecture 
Designed and tested on `aarch64` using QEMU (`virt` machine).

## Libraries Used:
* **fs.h** For VFS, so I can perform file operations.
* **interrupt.h** For interrupt API's I have used `irqreturn_t sensor_isr()` this helps me trigger the interrupts when `DUMMY_SENSOR_TRIGGER`
	macro is called.
	
	```c
	static irqreturn_t sensor_isr(int irq, void *dev_id) {
		printk(KERN_INFO "DummySensor: [Top Half] Hardware IRQ Fired! Deferring work...\n");
	
		schedule_work(&sensor_work);
	
		return IRQ_HANDLED;
	}
	```
	
* **miscdevice.h** Helps to load my module into `/dev/dummy_sensor`.

	```c
	
	static struct miscdevice sensor_misc = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = "dummy_sensor",
		.fops = &sensor_fops,
	};

	misc_register(&sensor_misc); // In sensor_probe
	misc_deregister(&sensor_misc); // In sensor_remove
	```

* **mod_devicetable.h** The function `of_table_id()` helps to get the particular sensor, I forcefully added the `kernelmentor,dummy_sensor`

	```c
	static const struct of_device_id sensor_dt_match[] = {
		{.compatible = "kernelmentor,dummy-sensor"},
		{/* Sentinel (must be NULL terminated)  */},
	};
	MODULE_DEVICE_TABLE(of, sensor_dt_match);
	```

* **module.h**
* **of.h**
* **platform_device.h** Helps to get the physical deivces that are sent by device_tree dtb.
* **uaccess.h** Used `copy_to_user()`, this helps to copy from kernel to userspace.
* **minmax.h** Used `min()` this gives the mid value between `count` and `len-*ppos`
	
	```c
	int bytes_to_copy = min((size_t)count, (size_t)len - *ppos);
	```	
* **string.h** Used strlen to get the length of the `buf`
* **slab.h**
* **mutex.h** Locks and Unlocks the sensor and interrupts to avoid race conditions.

	```c
	mutex_lock(&sensor_lock);		
	mutex_unlock(&sensor_lock);
	
	mutex_lock_interruptible(&sensor_lock);
	mutex_unlock_interruptible(&sensor_lock);
	```

* **workqueue.h** 
* **delay.h** Used `msleep()` for CPU simulation.
* **dummy_sensor_ioctl.h** custom created library that set and reset the dummy_sensor `sample_rate()` the default is 50.

	```c
	
	// Magic Number
	#define DUMMY_MAGIC 'd'

	// _IO means no data transfer just trigger the action
	#define DUMMY_SENSOR_RESET _IO(DUMMY_MAGIC, 1)

	// _IOW means the user is writing to the kernel
	#define DUMMY_SENSOR_SET_RATE _IOW(DUMMY_MAGIC, 2, int)

	// Manual Trigger due to missing debug file in /sys/kernel/
	// THE SOFTWARE TRIGGER
	#define DUMMY_SENSOR_TRIGGER_IRQ _IO(DUMMY_MAGIC, 3)
	```
