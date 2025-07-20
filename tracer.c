#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/user.h>
#include "syscall.h"

static void tracer_handler(int child_pid)
{
	struct user_regs_struct regs;
	int wstatus;

	while (1) {
		__sys_waitpid(child_pid, &wstatus, 0);
		assert(WIFSTOPPED(wstatus));
		assert(WSTOPSIG(wstatus) == SIGTRAP);

		__sys_ptrace(
			PTRACE_GETREGS, child_pid, NULL, &regs
		);
		printf("the child perform system call: %ld\n", regs.orig_rax);

		__sys_ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL);
	}
}

static void tracee_handler(const char *argv[], const char *envp[])
{	
	int ret = __sys_ptrace(PTRACE_TRACEME, 0, NULL, NULL);
	if (ret < 0) {
		fprintf(stderr, "failed to trace: %s\n", strerror(ret));
		return;
	}

	ret = __sys_execve(argv[1], &argv[1], envp);
	if (ret < 0) {
		fprintf(
			stderr,
			"failed to create new process image: %s\n",
			strerror(-ret)
		);
		return;
	}

}

int main(int argc, const char *argv[], const char *envp[])
{
	pid_t child_pid;

	if (argc < 2) {
		fprintf(stderr, "usage: %s <program path>\n", argv[0]);
		return -1;
	}

	child_pid = __sys_fork();
	if (child_pid == 0)
		tracee_handler(argv, envp);
	else
		tracer_handler(child_pid);

	return 0;
}
