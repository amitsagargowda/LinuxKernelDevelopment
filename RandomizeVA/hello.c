// sudo sh -c 'echo 0 > /proc/sys/kernel/randomize_va_space' 

#include <stdio.h>

int main(void)
{
	printf("Hello World! %p\n",&main);
//	while(1);
	return 0;
}
