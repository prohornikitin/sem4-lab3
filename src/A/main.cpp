#define _POSIX_SOURCE
#include <unistd.h>
#include <signal.h>
#include <aio.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cstdio>
#include <climits>


using std::string, std::cerr;


void exec_child(const char* filename) {
	if(execl(filename, filename, NULL) == -1) {
		perror("Can't execv() ./B");
		exit(1);
	}
}


void start_child(const char* filename) {
	pid_t pid = fork();
	if(pid > 0) {
		return;
	}
	if(pid < 0) {
		perror("fork() error");
		exit(1);
	}
	exec_child(filename);
}


void set_signal_ignore(int signal) {
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(signal, &sa, 0) == -1) {
		perror("sigaction() error");
		exit(1);
	}
}


void handle_sigusr1(int _) {

}


void parse_cmd_args(int argc, char * argv[],
	string & child_executable, unsigned & wait_timeout
) {
	if(argc != 3) {
		printf("This program must have exactly two arguments\n");
		exit(1);
	}

	child_executable = argv[1];

	unsigned long wait_timeout_ul;
	char* end_ptr;
	wait_timeout_ul = strtoul(argv[2], &end_ptr, 0);
	if(end_ptr != argv[2] + strlen(argv[2])) {
		cerr << "error: can't parse wait_timeout parameter\n";
		exit(1);
	}
	if(wait_timeout_ul == ULONG_MAX) {
		perror("wait_timeout parameter is out of range");
		exit(1);
	}
	if(wait_timeout_ul > UINT_MAX) {
		cerr << "wait_timeout parameter is out of range\n";
		exit(1);
	}
	wait_timeout = wait_timeout_ul;
}

int main(int argc, char * argv[]) {
	string child_executable;
	unsigned wait_timeout;
	parse_cmd_args(argc, argv,
		child_executable,
		wait_timeout
	);


	setsid();
	prctl(PR_SET_CHILD_SUBREAPER, 1);

	signal(SIGCHLD, SIG_IGN);
	signal(SIGUSR1, handle_sigusr1);

	start_child(child_executable.c_str());

	pause();
	sleep(wait_timeout);
	set_signal_ignore(SIGTERM);
	kill(-getpid(), SIGTERM);


	return 0;
}
