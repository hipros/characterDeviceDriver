#include<sys/stat.h>
#include<sys/ioctl.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>

#define DEVICE_FILE_NAME "/dev/BufferedMem"

int main (int argc, char *argv[])
{
	int device;
	int m = atoi(argv[1]);

	device = open(DEVICE_FILE_NAME, O_RDWR);

	if (device >= 0) {
		ioctl(device, 1, m);
	}
	else perror("Device file open fail");

	close(device);
	return 0;
}
