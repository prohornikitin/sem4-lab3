#define _POSIX_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stack>
#include <list>
#include <vector>

#include <input_parse.h>

using std::string, std::cerr, std::stack, std::list, std::vector;


list<string> tokenize(string & str) {
	list<string> tokens;
	size_t token_start = 0;

	stack<char> quotes;
	bool next_is_escaped = false;

	for(size_t i = 0; i < str.size(); ++i) {
		switch(str[i]) {
		case '\\':
			next_is_escaped = true;
			break;
		case '\'':
		case '"':
			if(next_is_escaped) {
				next_is_escaped = false;
				break;
			}
			if(quotes.empty() || quotes.top() != str[i]) {
				quotes.push(str[i]);
			} else {
				quotes.pop();
			}
			break;
		case ' ':
			if(next_is_escaped) {
				next_is_escaped = false;
				break;
			}
			if(quotes.empty()) {
				tokens.push_back(str.substr(token_start, i - token_start));
				token_start = i+1;
			}
			break;
		}
	}
	if(!quotes.empty()) {
		cerr << "error: quotes does not match\n";
		exit(1);
	}
	tokens.push_back(str.substr(token_start));

	for(auto & t : tokens) {
		size_t pos = 0;
		while((pos = t.find_first_of("'\"", pos)) != string::npos) {
			if(t[pos-1] != '\\') {
				t.erase(pos, 1);
			}
		}
	}
	return tokens;
}


Prog::Prog()
 : executable_(nullptr), args_(nullptr), args_count_(0) {}


Prog::Prog(const string & executable, const vector<string> & args) {
	executable_ = new char [executable.size()+1];
	strcpy(executable_, executable.c_str());

	args_count_ = args.size();
	args_ = new char* [args.size()];
	for(size_t i = 0; i < args.size(); ++i) {
		args_[i] = new char [args[i].size()+1];
		strcpy(args_[i], args[i].c_str());
	}
}


Prog::Prog(Prog && other) {
	args_ = other.args_;
	executable_ = other.executable_;
	args_count_ = other.args_count_;

	other.executable_ = nullptr;
	other.args_ = nullptr;
	other.args_count_ = 0;
}


void Prog::execute() {
	execvp(executable_, args_);
}


Prog::~Prog() {
	delete[] executable_;
	for(size_t i = 0; i < args_count_; ++i) {
		delete[] args_[i];
	}
	delete[] args_;
}


vector<Prog> parse(const list<string> & tokens) {
	enum class Type {
		PIPE,
		EXEC,
		ARG,
	};
	Type prev = Type::PIPE;

	vector<Prog> progs;
	string executable;
	vector<string> args;
	for(auto & t : tokens) {
		if(t == "|") {
			progs.push_back(Prog(executable, args));
			args.clear();
			prev = Type::PIPE;
			continue;
		}
		if(prev == Type::PIPE) {
			executable = t;
			prev = Type::EXEC;
		}
		if(prev == Type::EXEC || prev == Type::ARG) {
			args.push_back(t);
			prev = Type::ARG;
		}
	}
	progs.push_back(Prog(executable, args));
	return progs;
}
