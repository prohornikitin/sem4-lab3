#define _POSIX_SOURCE
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stack>
#include <list>
#include <vector>

#include <input_parse.h>

using std::cin, std::getline, std::string, std::cout, std::cerr, std::endl, std::vector;


void execute(vector<Prog> & progs) {
	int stdout = dup(STDOUT_FILENO);
	for(size_t i = 0; i < progs.size()-1; ++i) {
		int fd[2];
		pipe(fd);
		pid_t pid = fork();
		if(pid == -1) {
			perror("fork()");
			exit(1);
		}
		if(pid != 0) { //parent
			close(fd[0]);
			if(dup2(fd[1], STDOUT_FILENO) == -1) {
				perror("map stdout to the pipe with dup2()");
				exit(1);
			}
			close(fd[1]);
			progs[i].execute();
		}
		close(fd[1]);
		if(dup2(fd[0], STDIN_FILENO) == -1) {
			perror("map stdin to the pipe with dup2()");
			exit(1);
		}
		close(fd[0]);
	}

	if(!progs.empty()) {
		if(dup2(stdout, STDOUT_FILENO) == -1) {
			perror("map stdout to the terminal out with dup2()");
			exit(1);
		}
		progs[progs.size() - 1].execute();
	}
}


int main() {
	string input;
	getline(cin, input);

	kill(getppid(), SIGUSR1);

	auto tokens = tokenize(input);
	auto progs = parse(tokens);
	execute(progs);

	return 0;
}
