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
	
	if(m < 2) {
		printf("Too small value\n");
		return 0;
	}

	device = open(DEVICE_FILE_NAME, O_RDWR);

	if (device >= 0) {
		ioctl(device, 0, m);
	}
	else perror("Device file open fail");

	close(device);
	return 0;
}
