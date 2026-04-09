# Dummy Sensor Platform Driver

This is an complete, end-to-end Linux Platform Driver written for an ARM64 architecture.

## Features:
* **Device Tree Integration** Binds to a custom `Kernelmentor,dummy-sensor` node.
* **IRQ Handling** Maps and registers hardware interrupts using GIC.
* **VFS Data Pipeline** Exposes a Character Device node (`/dev/dummy-sensor`) for user-space data reads.
* **Sysfs Control Panel** Exposes a configurable `sample_rate` attribute in `/sys/bus/platform/devices`.

## Architecture 
Designed and tested on `aarch64` using QEMU (`virt` machine).

## 1. Concurrency & Race Condition Defense (SMP)

Modern Linux utilizes Symmetric Multi-Processing (SMP), meaning multiple CPU cores execute tasks in parallel.
This creates a vulnerability: if multiple user-space programs attempt to access or modify shared kernel variables (like the sample_rate) 
at the exact same nanosecond, it causes a race condition that corrupts the data.

**TO PERVENT THIS**

I implemented a Mutex data structure as a defense mechanism. 
By wrapping critical sections of code with mutex_lock_interruptible() (for the IOCTL pipeline) and mutex_lock() (for the Sysfs interface), 
the driver ensures exclusive access. When one CPU is interacting with the sensor data, the Mutex locks the buffer, 
ensuring no other CPU can intervene until the operation is complete and the lock is released.


## 2. Deferred Interrupt Processing (Top Half and Bottom Half)
When a hardware interrupt occurs, the CPU halts its current execution and jumps into an Interrupt Service Routine (ISR). 
Because the system often disables other interrupts while inside an ISR (operating in an atomic context), 
performing heavy tasks here will freeze the operating system, leading to system lag and 
dropped events (like keyboard inputs or network packets).

To solve this, I engineered an asynchronous, deferred processing architecture to keep the CPU responsive:

The Top Half (Hard IRQ): The sensor_isr() function executes instantly. Its only job is to acknowledge the hardware 
trigger, toss the heavy processing task to the kernel using the schedule_work() API, and exit immediately.

The Bottom Half (Workqueue): A dedicated struct work_struct runs asynchronously in the background. 
Because it executes in a "process context," it can safely process the data, utilize msleep() for CPU simulation, 
and wait for Mutex locks without blocking the core operating system.
