# Dummy Sensor Platform Driver

This is an complete, end-to-end Linux Platform Driver written for an ARM64 architecture.

## Features:
* **Device Tree Integration** Binds to a custom `Kernelmentor,dummy-sensor` node.
* **IRQ Handling** Maps and registers hardware interrupts using GIC.
* **VFS Data Pipeline** Exposes a Character Device node (`/dev/dummy-sensor`) for user-space data reads.
* **Sysfs Control Panel** Exposes a configurable `sample_rate` attribute in `/sys/bus/platform/devices`.

## Architecture 
Designed and tested on `aarch64` using QEMU (`virt` machine).
