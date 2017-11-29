#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
int main()
{
	int result;
	int number1 = 5;
	int number2 = 8;

	syscall(333, number1, number2, &result);


        printf("%d + %d = %d\n",number1, number2, result);
	return 0;
}
