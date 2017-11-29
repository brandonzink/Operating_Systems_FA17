#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#define DEVICE "/dev/simple_char_device"
#define BUFFER_SIZE 1024

int main () {
	char command; //command
	char buffer[BUFFER_SIZE]; //bytes to write
	char* buff; //memory allocate
	int offset = 0; 
	int whence = 0; //llseek
	int file = open(DEVICE, O_RDWR); //open the file with read/write access
	loff_t position = 0;
	while (1) { 
		printf("\n-----MENU-----\n\nr or R) Read from device\nw or W) Write to device\ns or S) Seek from device\ne or E) Exit device\n\nEnter command: ");
		scanf("%c", &command);
		while(getchar() != '\n'); //added me
		switch (command) {
			case 'w': 
			case 'W':
				printf("Enter data you want to write to the device: ");
				scanf("%s", buffer); //might be %c
				write(file, buffer, BUFFER_SIZE, &position);
				while (getchar() != '\n'); //makes it so that enter is not part of input
				break;
			case 'r': 
			case 'R':
				printf("Enter the number of bytes you want to read: ");
				scanf("%d", buffer);
				//makes the buffer
				buff = (char *)malloc(buffer);
				read(file, buff, &buffer, &position);
				printf("Device output: %s\n", buffer);
				while (getchar() != '\n'); //makes it so that enter is not part of input
				break;
			case 's':
			case 'S':
				printf("Enter an offset value:\n");
				scanf("%d", &offset); //might not be %c
				//while (getchar() != '\n'); //makes it so that enter is not part of input
				printf("Enter a value for whence:\n");
				scanf("%d", &whence);
				//while (getchar() != '\n'); //makes it so that enter is not part of input
				position = llseek(file, offset, whence);
				while (getchar() != '\n'); //makes it so that enter is not part of input
				break;
			case 'e':
			case 'E':
				return 0;

			default:
				while (getchar() != '\n'); //makes it so that enter is not part of input
		}
	}
	close(file);
	return 0;
}