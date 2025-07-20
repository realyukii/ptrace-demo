#include <stdio.h>
#include <time.h>
#include <sys/wait.h>
#include "syscall.h"

int main(int argc, char *argv[])
{
	pid_t pid;
	pid = __sys_fork();
	if (pid == 0) {
		struct timespec ts = {
			.tv_sec = 5
		};
		__sys_nanosleep(&ts, NULL);
		printf("child pid: %d\n", __sys_getpid());
		return 5;
	} else {
		int wstatus;
		printf("parent pid: %d\n", __sys_getpid());
		__sys_waitpid(pid, &wstatus, 0);
		printf("process with pid %d exited with status %d\n", pid, WEXITSTATUS(wstatus));
	}

	return 6;
}
