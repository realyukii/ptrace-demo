#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/user.h>
#include <errno.h>
#include <Zydis/Zydis.h>
#include "syscall.h"

static void tracer_handler(int child_pid)
{
	ZydisDisassembledInstruction instruction;
	struct user_regs_struct regs;
	long ret, insn;
	int wstatus;

	while (1) {
		__sys_waitpid(child_pid, &wstatus, 0);
		if (WIFEXITED(wstatus)) {
			__sys_write(STDOUT_FILENO, "child process exited\n", 21);
			break;
		}

		assert(WIFSTOPPED(wstatus));
		assert(WSTOPSIG(wstatus) == SIGTRAP);

		__sys_ptrace(
			PTRACE_GETREGS, child_pid, NULL, &regs
		);

		/*
		* From the manual page:
		* > C library/kernel differences
		* > At the system call level, the PTRACE_PEEKTEXT, PTRACE_PEEKDATA,
		* > and PTRACE_PEEKUSER operations have a different API: they store the result
		* > at the address specified by the data  parameter, and the return value is the error flag.
		* > The glibc wrapper function provides the API given in DESCRIPTION above, with the result being returned
		* > via the function return value.
		*/
		insn = 0;
		ret = __sys_ptrace(
			PTRACE_PEEKTEXT, child_pid, (void *)regs.rip, &insn
		);
		assert(!ret);

		if (ZYAN_SUCCESS(ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, regs.rip, &insn, sizeof(insn), &instruction)))
			printf("%p: %s\n", (void *)regs.rip, instruction.text);

		__sys_ptrace(PTRACE_SINGLESTEP, child_pid, NULL, NULL);
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
