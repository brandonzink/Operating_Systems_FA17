#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
int main()
{
         long int amma = syscall(332);
         printf("HELLOWOLRD RETURNED %ld\n", amma);
         return 0;
}
