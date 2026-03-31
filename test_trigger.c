#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "dummy_sensor_ioctl.h"

int main()
{
	int fd = open("/dev/dummy_sensor", O_RDWR);
	if (fd < 0) {
		perror("Failed to open /dev/dummy_sensor:");
		return -1;
	}

	printf("Firing Software interrupt trigger..\n");
	ioctl(fd, DUMMY_SENSOR_TRIGGER_IRQ);
	
	close(fd);
	return 0;
}
