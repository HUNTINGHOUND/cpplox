#include "memory.hpp"
#include "value.hpp"
#include "chunk.hpp"

void* reallocate(void* pointer, size_t oldsize, size_t newsize) {
    if (newsize == 0) {
        free(pointer);
        return nullptr;
    }
    
    void* result = realloc(pointer, newsize);
    return result;
}

