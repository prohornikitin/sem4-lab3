#include <list>
#include <vector>


class Prog {
public:
	Prog();
	Prog(const std::string & executable, const std::vector<std::string> & args);
	Prog(const Prog & b) = delete;
	Prog(Prog && other);

	void execute();
	~Prog();

private:
	char* executable_;
	char** args_;
	size_t args_count_;
};


std::list<std::string> tokenize(std::string & str);
std::vector<Prog> parse(const std::list<std::string> & tokens);


