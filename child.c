#include "syscall.h"

static const char buff[] = "test\n";

int main(void)
{
	__sys_write(STDOUT_FILENO, buff, sizeof(buff));

	return 0;
}
