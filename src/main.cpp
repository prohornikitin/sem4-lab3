#include <alloc.h>
#include <thread>
#include <list>
#include <array>
#include <vector>
#include <fstream>
#include <sstream>

using std::list, std::thread, std::array, std::vector, std::ofstream, std::ostream, std::string, std::stringstream;

template <class T>
class MemoryBlock {
public:
	void allocate(size_t count) {
		_address = (T*)my_malloc(count * sizeof(T));
		_count = count;
	}

	T& operator[](size_t index) {
		return _address[index];
	}

	bool is_allocated() const {
		return (_address != nullptr);
	}

	void fill_with(T fill) {
		std::fill(_address, _address + _count, fill);
	}

	void free() {
		my_free(_address);
		_address = nullptr;
	}

	size_t * address() {
		return _address;
	}

	size_t count() {
		return _count;
	}

private:
	T * _address = nullptr;
	size_t _count = 0;
};



struct SharedData {
public:
	enum class Operation {
		ALLOCATION,
		FILL,
		FREE,
		NONE,
	};

	/* After function finishes mutex will be still locked! */
	void wait_until_next(SharedData::Operation next) {
		using namespace std::chrono_literals;
		while(true) {
			mutex.lock();
			if(next_operation == next) {
				break;
			}
			mutex.unlock();
			std::this_thread::sleep_for(100ms);
		}
	}

	array<MemoryBlock<size_t>, 3> blocks;
	std::mutex mutex;
	Operation next_operation = Operation::ALLOCATION;
};


void alloc_thread(SharedData & data) {
	data.wait_until_next(SharedData::Operation::ALLOCATION);
	auto& blocks = data.blocks;
	blocks[0].allocate(4);
	blocks[1].allocate(256);
	blocks[2].allocate(1024*256);
	printf(
		"%p %p %p\n",
		(void*)blocks[0].address(),
		(void*)blocks[1].address(),
		(void*)blocks[2].address()
	);
	data.next_operation = SharedData::Operation::FILL;
	data.mutex.unlock();
}



void fill_thread(SharedData & data, size_t n) {
	data.wait_until_next(SharedData::Operation::FILL);
	for(size_t i = 0; i < 3; ++i) {
		data.blocks[i].fill_with(n);
	}
	data.next_operation = SharedData::Operation::FREE;
	data.mutex.unlock();
}



void free_thread(SharedData & data, string filename) {
	ofstream out(filename);
	data.wait_until_next(SharedData::Operation::FREE);
	for(size_t i = 0; i < 3; ++i) {
		for(size_t j = 0; j < data.blocks[i].count(); ++j) {
			out << data.blocks[i][j] << " ";
		}
		out << "\n";
		out.flush();
		data.blocks[i].free();
	}
	data.next_operation = SharedData::Operation::NONE;
	data.mutex.unlock();
}





int main() {
	const size_t N = 3 * 11;

	vector<SharedData> data(N/3);
	list<thread> threads;
	
	for(size_t i = 0; i < N/3; i++) {
		threads.emplace_back(alloc_thread, std::ref(data[i]));
	}

	for(size_t i = 0; i < N/3; i++) {
		threads.emplace_back(fill_thread,std::ref(data[i]), i+N/3);
	}

	for(size_t i = 0; i < N/3; i++) {
		stringstream filename;
		filename << "out" << i << ".txt";
		threads.emplace_back(free_thread, std::ref(data[i]), filename.str());
	}

	while(threads.size() != 0) {
		threads.front().join();
		threads.pop_front();
	}
}
