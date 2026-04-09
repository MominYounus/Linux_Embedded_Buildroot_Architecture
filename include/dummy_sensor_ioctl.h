#ifndef DUMMY_SENSOR_IOCTL_H
#define DUMMY_SENSOR_IOCTL_H

#include <linux/ioctl.h>

// Magic Number
#define DUMMY_MAGIC 'd'

// _IO means no data transfer just trigger the action
#define DUMMY_SENSOR_RESET _IO(DUMMY_MAGIC, 1)

// _IOW means the user is writing to the kernel
#define DUMMY_SENSOR_SET_RATE _IOW(DUMMY_MAGIC, 2, int)

// Manual Trigger due to missing debug file in /sys/kernel/
// THE SOFTWARE TRIGGER
#define DUMMY_SENSOR_TRIGGER_IRQ _IO(DUMMY_MAGIC, 3)

#endif
