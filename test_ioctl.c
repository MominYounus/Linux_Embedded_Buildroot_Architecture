#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "dummy_sensor_ioctl.h"

int main()
{
	int fd;
	int rate = 999;

	printf("Opening /dev/dummy_sensor..\n");
	fd = open("/dev/dummy_sensor", O_RDWR);
	if (fd < 0) {
		perror("Failed to open device");
		return -1;
	}

	printf("Sending SET_RATE command (Rate: %d)..\n", rate);
	
	if(ioctl(fd, DUMMY_SENSOR_SET_RATE, &rate) < 0)
		perror("Failed to set rate");

	printf("Sending RESET command...\n");
	if (ioctl(fd, DUMMY_SENSOR_RESET) < 0)
		perror("Failed to reset");
	
	close(fd);
	return 0;
}
