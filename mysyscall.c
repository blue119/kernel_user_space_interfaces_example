#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#define MYSYSCALL 325

int main(){
	char *buf [20];
	memcpy(buf, "hi kernel", strlen("hi kernel") +1);
	syscall(MYSYSCALL, buf, 10);
	printf("kernel said %s\n", buf);
	return 0;
}

