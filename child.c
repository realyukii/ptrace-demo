#include "syscall.h"

int main(void)
{
	static const char buff[] = "test\n";
	__sys_write(STDOUT_FILENO, buff, sizeof(buff));
	return 0;
}
