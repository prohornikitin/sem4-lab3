#include <cstddef>


void * my_malloc(size_t size);

void * my_calloc(size_t num, size_t size);

void * my_realloc(void * ptr, size_t new_size);

void my_free(void * ptr);
