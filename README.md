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
