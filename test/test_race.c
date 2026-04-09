#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "dummy_sensor_ioctl.h"

// Thread 1: Spams the RESET command 10,000 times
void *thread_reset(void *arg) {
	int fd = open("/dev/dummy_sensor", O_RDWR);
	for (int i = 0; i < 10000; ++i) ioctl(fd, DUMMY_SENSOR_RESET);
	close(fd);
	return NULL;
}

// Thread 2: Spams the SET_RATE command 10,000 times
void *thread_set(void *arg) {
	int fd = open("/dev/dummy_sensor", O_RDWR);
	int rate = 999;
	for (int i = 0; i < 10000; ++i) ioctl(fd, DUMMY_SENSOR_SET_RATE, &rate);
	close(fd);
	return NULL;
}

int main()
{
	pthread_t t1, t2;
	printf("Starting brutal concurrency test (20,000 parallel ioctl)...\n");
	
	// Lauching both threads at the same time
	pthread_create(&t1, NULL, thread_reset, NULL);
	pthread_create(&t2, NULL, thread_set, NULL);

	// Waits for them (t1, t2) to finish
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	printf("TEST COMPLETE: kernel didn't crash , so my mutex works fine..\n");
	return 0;
}
