#include "memory.hpp"
#include "value.hpp"
#include "chunk.hpp"

template <typename T>
void reallocate(std::vector<T>& array, size_t oldsize, size_t newsize) {
    if (newsize == 0) {
        array.clear();
        std::vector<T>().swap(array);
        return;
    }
    
    array.resize(newsize);
}

template void reallocate<uint8_t>(std::vector<uint8_t>&, size_t, size_t);
template void reallocate<Value>(std::vector<Value>&, size_t, size_t);
template void reallocate<Line>(std::vector<Line>&, size_t, size_t);
