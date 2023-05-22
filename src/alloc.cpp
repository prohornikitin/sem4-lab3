#include <alloc.h>
#include <cstdlib>
#include <mutex>
#include <memory>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>


template <class T>
T * align(void * addr, size_t space) {
	return (T *)std::align(alignof(T), sizeof(T), addr, space);
}

struct MmappedAreasPage;

static MmappedAreasPage * first_page = nullptr;
static long page_size = 0;
static std::mutex mutex;


struct Block {
	size_t capacity;
	bool is_free = 0;
	char data[];
};

struct MmappedArea {
	void* address = nullptr;
	size_t pages_count = 0;
};


struct MmappedAreasPage {
	MmappedAreasPage * next = nullptr;
	size_t areas_count = 0;
	MmappedArea areas[];

	size_t max_areas() {
		return (page_size - offsetof(MmappedAreasPage, areas)) / sizeof(MmappedArea);
	}
};




void * mmap_wrapper(size_t pages) {
	return mmap(NULL, pages * page_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
}

void init() {
	page_size = sysconf(_SC_PAGESIZE);
	first_page = (MmappedAreasPage *) mmap_wrapper(1);
}


MmappedArea * allocate_area(size_t pages) {
	MmappedAreasPage * i = first_page;
	MmappedAreasPage * prev = first_page;
	while(i != nullptr) {
		if(i->max_areas() > i->areas_count) {
			break;
		}
		prev = i;
		i = i->next;
	}
	if(i == nullptr) {
		i = prev->next = (MmappedAreasPage *) mmap_wrapper(1);
	}
	auto area = &i->areas[i->areas_count];
	area->pages_count = pages;
	area->address = mmap_wrapper(pages);
	if(area->address == nullptr) {
		return nullptr;
	}
	i->areas_count++;
	return area;
}






void mark_up_newly_allocated_area(MmappedArea * area, size_t size) {
	size_t real_size = area->pages_count * page_size;
	Block * first_block = (Block *) area->address;
	first_block->is_free = false;
	size_t remains = real_size - sizeof(Block) - size;
	Block * last_block = align<Block>(&first_block->data[size], remains);
	if(last_block == nullptr) {
		first_block->capacity = remains + size;
		return;
	}
	first_block->capacity = size;
	last_block->capacity = ((char *)area + real_size - (char *)&last_block->data[0]);
	last_block->is_free = 1;
}

Block * request_block_from_os(size_t size) {
	size_t pages = (size + sizeof(Block) + page_size - 1) / page_size;
	MmappedArea * area = allocate_area(pages);
	mark_up_newly_allocated_area(area, size);
	return (Block *)area->address;
}

Block * try_find_block(size_t size) {
	for(auto p = first_page; p != nullptr; p = p->next) {
		for(size_t i = 0; i < p->areas_count; i++) {
			auto area = &p->areas[i];
			Block * b = (Block*)(area->address);
			char* end = (char*)(area->address) + area->pages_count * page_size;
			while((end - (char*)b) < sizeof(Block)) {
				if(b->is_free && b->capacity >= size) {
					//todo: split
					b->is_free = 0;
					return b;
				}
				b = (Block *)(b->data + b->capacity);
			}
		}
	}
	return nullptr;
}

void * my_malloc(size_t size) {
	mutex.lock();
	if(first_page == nullptr) {
		init();
	}
	Block * block = try_find_block(size);
	if(block != nullptr) {
		mutex.unlock();
		return block->data;
	}
	block = request_block_from_os(size);
	if(block != nullptr) {
		mutex.unlock();
		return block->data;
	}
	mutex.unlock();
	return nullptr;
}


void * my_calloc(size_t num, size_t size) {
	void* memory = my_malloc(size * num);
	memset(memory, 0, size * num);
	return memory;
}

void * my_realloc(void * ptr, size_t new_size) {
	//todo
}


void find_by_ptr(void * ptr, MmappedAreasPage *& page, MmappedArea *& area) {
	printf("page=%p", first_page);
	for(page = first_page; page != nullptr; page = page->next) {
		for(size_t i = 0; i < page->areas_count; i++) {
			area = &page->areas[i];
			void * start = area->address;
			void * end = (void *)(((char *)area->address) + area->pages_count * page_size);
			if(start < ptr && ptr < end) {
				return;
			}
		}
	}
	page = nullptr;
	area = nullptr;
}


void my_free(void * ptr) {
	MmappedAreasPage * page;
	MmappedArea * area;
	find_by_ptr(ptr, page, area);
	printf("ptr=%p, page=%p, area=%p\n", ptr, page, area);
	if(page == nullptr) {
		printf("Error: cannot free memory which was not allocated\n");
	}
	
}
