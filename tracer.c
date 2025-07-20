#include <stdio.h>
#include "syscall.h"

int main(int argc, char *argv[])
{
	intptr_t rip;
	pid_t pid;

	pid = __sys_fork();
	if (pid == 0) {
		struct timespec ts = {
			.tv_sec = 5
		};

		__sys_nanosleep(&ts, NULL);

		asm volatile ("1: lea 1b(%%rip), %0" : "=r" (rip));

		printf("child pid=%d | rip=%p\n", __sys_getpid(), rip);

		return 5;
	} else {
		int wstatus;

		asm volatile ("1: lea 1b(%%rip), %0" : "=r" (rip));
	
		printf("parent pid=%d | rip=%p\n", __sys_getpid(), rip);
		__sys_waitpid(pid, &wstatus, 0);
		printf("process with pid %d exited with status %d\n", pid, WEXITSTATUS(wstatus));
	}

	return 6;
}
