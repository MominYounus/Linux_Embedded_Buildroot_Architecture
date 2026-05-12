#include <linux/module.h>
#include <linux/i2c.h>

static int mpu6050_probe(struct i2c_client *client) {
	printk(KERN_INFO "Successfully Detected MPU6050 at 0x%02x\n", client->addr);
	return 0;
}

static void mpu6050_remove(struct i2c_client *) {
	printk(KERN_INFO "Removed MPU6050\n");
}

static const struct i2c_device_id mpu_sensor_id[] = {
	{"mpu6050", 0},
	{},
};

static struct i2c_driver mpu6050_driver = {
	.driver = {
		.name = "mpu6050",
	},
	.probe = mpu6050_probe,
	.remove = mpu6050_remove,
	.id_table = mpu_sensor_id,
};

module_i2c_driver(mpu6050_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohammed Momin");
MODULE_DESCRIPTION("MPU6050 I2C Client Skeleton Driver");
