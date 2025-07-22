#define _GNU_SOURCE
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/user.h>
#include <errno.h>
#include <Zydis/Zydis.h>
#include "syscall.h"

struct GWTracer_Cfg {
	pid_t pid;
	char *file_path;
	char **sub_argv;
};

static uint8_t must_stop = 0;

static const char usage[] = "usage: %s [options]\n"
"-f, --file-path\t<path to executable program>\n"
"-p, --pid\t<process id>\n"
"--\t<sub program option(s)>\n\n"
"note:\n"
"-- is only useful for --path and want to pass cmdline arguments for it\n";

struct GWTracer_Cfg *global_cfg;
static void signal_handler(int c)
{
	(void)c;
	must_stop = 1;
	__sys_ptrace(PTRACE_DETACH, global_cfg->pid, NULL, NULL);
}

#define copy_from_process(pid, dst, src, len) \
	__copy_from_process((pid), (dst), (const void *)(src), (len))

static ssize_t __copy_from_process(int pid, void *dst, const void *src, size_t len)
{
	const char *srcp = src;
	char *dstp = dst;

	while (len > 0) {
		size_t cp_len;
		ssize_t ret;
		long tmp;

		ret = __sys_ptrace(PTRACE_PEEKTEXT, pid, (void *)srcp, &tmp);
		if (ret < 0)
			return ret;

		cp_len = (len > sizeof(tmp)) ? sizeof(tmp) : len;
		memcpy(dstp, &tmp, cp_len);
		srcp += cp_len;
		dstp += cp_len;
		len -= cp_len;
	}

	return 0;
}

static void tracer_handler(int child_pid)
{
	ZydisDisassembledInstruction instruction;
	struct user_regs_struct regs;
	long ret, insn[2];
	int wstatus;

	struct sigaction sa = {
		.sa_handler = signal_handler
	};
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	while (1) {
		__sys_waitpid(child_pid, &wstatus, 0);
		if (must_stop)
			break;

		if (WIFEXITED(wstatus)) {
			__sys_write(STDOUT_FILENO, "child process exited\n", 21);
			break;
		}

		assert(WIFSTOPPED(wstatus));
		/*
		* we don't interested on signal beyond debugging (e.g SIGWINCH)
		*/
		// if (WSTOPSIG(wstatus) != SIGTRAP
		// && WSTOPSIG(wstatus) != SIGSTOP)
		// 	continue;

		__sys_ptrace(
			PTRACE_GETREGS, child_pid, NULL, &regs
		);

		copy_from_process(child_pid, insn, regs.rip, 4);

		if (!memcmp(insn, "\x83\x45\xc8\x01", 4)) {
			int counter;
			copy_from_process(child_pid, &counter, regs.rbp-0x38, sizeof(counter));
			printf("counter incremented: %d!\n", counter);
		}

		// insn[0] = 0;
		// ret = __sys_ptrace(
		// 	PTRACE_PEEKTEXT, child_pid,
		// 	(void *)regs.rip, &insn[0]
		// );
		// assert(!ret);

		// insn[1] = 0;
		// ret = __sys_ptrace(
		// 	PTRACE_PEEKTEXT, child_pid,
		// 	(void *)regs.rip + sizeof(long), &insn[1]
		// );
		// assert(!ret);

		// ret = ZydisDisassembleIntel(
		// 		ZYDIS_MACHINE_MODE_LONG_64, regs.rip, insn,
		// 		sizeof(insn), &instruction
		// );
		// if (ZYAN_SUCCESS(ret)) {
		// 	uint8_t len = instruction.info.length;

		// 	printf(
		// 		"%p: %s (instruction length %d) ",
		// 		(void *)regs.rip, instruction.text, len
		// 	);

		// 	for (size_t i = 0; i < len; i++) {
		// 		uint8_t *ptr = (void *)insn;
		// 		printf("0x%02x ", ptr[i]);
		// 	}

		// 	puts("");

		// } else {
		// 	/*
		// 	* unlikely.
		// 	* for error interpretation, see:
		// 	* https://github.com/zyantific/zydis/blob/master/tools/ZydisToolsShared.c#L79
		// 	*/
		// 	fprintf(stderr, "error occured: %d\n", ZYAN_STATUS_CODE(ret));
		// 	__sys_ptrace(PTRACE_DETACH, child_pid, NULL, NULL);
		// 	break;
		// }

		__sys_ptrace(PTRACE_SINGLESTEP, child_pid, NULL, NULL);
	}
}

static void tracee_handler(struct GWTracer_Cfg *cfg)
{
	int ret = __sys_ptrace(PTRACE_TRACEME, 0, NULL, NULL);
	if (ret < 0) {
		fprintf(stderr, "failed to trace: %s\n", strerror(ret));
		return;
	}

	ret = __sys_execve(cfg->file_path, (const char **)cfg->sub_argv, NULL);
	fprintf(
		stderr,
		"failed to create new process image: %s\n",
		strerror(-ret)
	);
}

static int parse_cmdline_args(struct GWTracer_Cfg *cfg, int argc, char * const *argv)
{
	int c;
	int opt_idx;
	char *pid_opt = NULL;
	char *path_opt = NULL;
	const struct option longopts[] = {
		{ "file-path", required_argument, NULL, 'f' },
		{ "pid", required_argument, NULL, 'p' },
		{ NULL, 0, NULL, 0 }
	};

	while (1) {
		c = getopt_long(argc, argv, "f:p:", longopts, &opt_idx);
		if (c == -1)
			break;

		switch (c) {
		case 'f':
			path_opt = optarg;
			break;
		case 'p':
			pid_opt = optarg;
			break;
		case '?':
		case ':':
			return -1;
		}
	}

	if (!pid_opt && !path_opt) {
		fprintf(stderr, usage, argv[0]);
		return -1;
	} else if (pid_opt && path_opt) {
		fprintf(stderr, "Use --pid OR --file-path, not both\n");
		return -1;
	}

	if (path_opt) {
		cfg->file_path = path_opt;
		int sub_argc = argc - optind;
		if (sub_argc > 0) {
			cfg->sub_argv = malloc((sub_argc + 2) * sizeof(char*));
			cfg->sub_argv[0] = path_opt;
			for (int i = 0; i < sub_argc; i++)
				cfg->sub_argv[i+1] = argv[optind + i];
			cfg->sub_argv[sub_argc + 1] = NULL;
		} else {
			cfg->sub_argv = malloc(2 * sizeof(char*));
			cfg->sub_argv[0] = path_opt;
			cfg->sub_argv[1] = NULL;
		}
	}

	if (pid_opt) {
		cfg->pid = atoi(pid_opt);
		if (cfg->pid <= 0) {
			fprintf(stderr, "invalid pid\n");
			return -1;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	struct GWTracer_Cfg cfg;
	global_cfg = &cfg;

	if (parse_cmdline_args(&cfg, argc, argv))
		return -1;

	if (cfg.file_path) {
		cfg.pid = __sys_fork();
		if (cfg.pid == 0)
			tracee_handler(&cfg);
		else {
			tracer_handler(cfg.pid);
			free(cfg.sub_argv);
		}
	} else {
		long ret = __sys_ptrace(PTRACE_ATTACH, cfg.pid, NULL, NULL);
		if (ret) {
			fprintf(stderr, "ptrace: %s\n", strerror(-ret));
			return -1;
		}

		tracer_handler(cfg.pid);
	}

	return 0;
}
