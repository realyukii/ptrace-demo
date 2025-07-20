#include <stdint.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifndef SYSCALL_H
#define SYSCALL_H

#ifdef __x86_64__
#define __do_syscall0(NUM) ({				\
	intptr_t ret;					\
	asm volatile (					\
		"syscall"				\
		: "=a" (ret)				\
		: "a" (NUM)		/* %rax */	\
		: "rcx", "r11", "memory"		\
	);						\
	ret;						\
})

#define __do_syscall2(NUM, ARG1, ARG2) ({		\
	intptr_t ret;					\
	asm volatile (					\
		"syscall"				\
		: "=a" (ret)				\
		: "a" (NUM),		/* %rax */	\
		  "D" (ARG1),		/* %rdi */	\
		  "S" (ARG2)		/* %rsi */	\
		: "rcx", "r11", "memory"		\
	);						\
	ret;						\
})

#define __do_syscall3(NUM, ARG1, ARG2, ARG3) ({		\
	intptr_t ret;					\
	asm volatile (					\
		"syscall"				\
		: "=a" (ret)				\
		: "a" (NUM),		/* %rax */	\
		  "D" (ARG1),		/* %rdi */	\
		  "S" (ARG2),		/* %rsi */	\
		  "d" (ARG3)		/* %rdx */	\
		: "rcx", "r11", "memory"		\
	);						\
	ret;						\
})

#define __do_syscall4(NUM, ARG1, ARG2, ARG3, ARG4) ({	\
	register void *_r10 asm("r10") = ARG4;		\
	intptr_t ret;					\
	asm volatile (					\
		"syscall"				\
		: "=a" (ret)				\
		: "a" (NUM),		/* %rax */	\
		  "D" (ARG1),		/* %rdi */	\
		  "S" (ARG2),		/* %rsi */	\
		  "d" (ARG3),		/* %rdx */	\
		  "r" (_r10)		/* %r10 */	\
		: "rcx", "r11", "memory"		\
	);						\
	ret;						\
})

ssize_t __sys_write(int fd, const void *buf, size_t count)
{
	return (ssize_t)__do_syscall3(__NR_write, fd, buf, count);
}

pid_t __sys_fork(void)
{
	return (pid_t)__do_syscall0(__NR_fork);
}

pid_t __sys_getpid(void)
{
	return (pid_t)__do_syscall0(__NR_getpid);
}

pid_t __sys_waitpid(pid_t pid, int *wstatus, int options)
{
	return (pid_t)__do_syscall3(__NR_wait4, pid, wstatus, options);
}

int __sys_nanosleep(const struct timespec *duration, struct timespec *rem)
{
	return (int)__do_syscall2(__NR_nanosleep, duration, rem);
}

int __sys_execve(const char *path, const char *argv[], const char *envp[])
{
	return (int)__do_syscall3(__NR_execve, path, argv, envp);
}

int __sys_ptrace(enum __ptrace_request op, pid_t pid, void *addr, void *data)
{
	return (long)__do_syscall4(__NR_ptrace, op, pid, addr, data);
}

#endif // __x86_64__

#endif // SYSCALL_H
